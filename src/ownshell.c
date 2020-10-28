#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int swap_dir(char **list) {
    char *sweet_home = getenv("HOME");
    if (!strcmp(list[0], "cd")) {
        if (list[1] == NULL || (!strcmp(list[1], "~"))) {
            chdir(sweet_home);
        } else {
            chdir(list[1]);
        }
        return 1;
    }
    return 0;
}

int cmp_exit(char *word) {
    int ans = (strcmp(word, "exit") && strcmp(word, "quit"));
    return ans;
}

void handler(int signo) {
    kill(0, SIGINT);
    printf("\nSIGINT received!\n");
    exit(1);
}

char *get_word(char *end) {
    char ch, *word = NULL;
    int len = 0;
    ch = getchar();
    word = malloc(sizeof(char));
    while (ch != ' ' && ch != '\n') {
        word = realloc(word, (len + 1) * sizeof(char));
        word[len] = ch;
        len++;
        ch = getchar();
    }
    *end = ch;
    word = realloc(word, (len + 1) * sizeof(char));
    word[len] = '\0';
    return word;
}

char **get_list() {
    char **list, end;
    int count = 1;
    list = malloc(sizeof(char*));
    if (!list) {
        return NULL;
    }
    list[0] = get_word(&end);
    while (end != '\n') {
        list = realloc(list, (count + 1) * sizeof(char*));
        list[count] = get_word(&end);
        count++;
    }
    list = realloc(list, (count + 1) * sizeof(char*));
    list[count] = NULL;
    return list;
}

void delete_list(char **list) {
    int i;
    if (list == NULL)
        return;
    for (i = 0; list[i] != NULL; i++) {
        free(list[i]);
    }
    free(list[i]);
    free(list);
    return;
}

int search(char **list, char *word) {
    int pos;
    for (pos = 0; list[pos] != NULL; pos++) {
        if (!strcmp(word, list[pos])) {
            return pos;
        }
    }
    return -1;
}

void io_file(char **list) {
    int pos1 = -1, pos2 = -1;
    int fd1 = 0, fd2 = 1;
    pos1 = search(list, "<");
    pos2 = search(list, ">");
    if (pos2 != -1) {
        fd2 = open(list[pos2 + 1], O_WRONLY | O_CREAT | O_TRUNC,
                                S_IRUSR | S_IWUSR);
        list[pos2] = NULL;
        dup2(fd2, 1);
        close(fd2);
    }
    if (pos1 != -1) {
        fd1 = open(list[pos1 + 1], O_RDONLY,
                                S_IRUSR | S_IWUSR);
        list[pos1] = NULL;
        dup2(fd1, 0);
        close(fd1);
    }
}

char ***great_divider(char **list, int *number, char *word) {
    int words = 0, count = 1, size = 0;
    char ***list_parts = malloc(sizeof(char**));
    list_parts[0] = NULL;
    if (!list_parts) {
        perror("devider erro!\n");
        return NULL;
    }
    for (int i = 0; list[i] != NULL; i++) {
        if (strcmp(list[i], word) != 0) {
            size = (words + 1) * sizeof(char*);
            list_parts[count - 1] = realloc(list_parts[count - 1], size);
            list_parts[count - 1][words] = list[i];
            words++;
        } else {
            size = (words + 1) * sizeof(char*);
            list_parts[count - 1] = realloc(list_parts[count - 1], size);
            list_parts[count - 1][words] = NULL;
            list_parts = realloc(list_parts, (count + 1) * sizeof(char**));
            list_parts[count] = NULL;
            count++;
            words = 0;
        }
    }
    size = (words + 1) * sizeof(char*);
    list_parts[count - 1] = realloc(list_parts[count - 1], size);
    list_parts[count - 1][words] = NULL;
    *number = count;
    return list_parts;
}

int implementation(char **list) {
    io_file(list);
    if (execvp(list[0], list) < 0) {
        perror("exec failed!\n");
        return -1;
    }
    return 0;
}

int conv1(char ***list_parts, int count) {
    int fd1[2] = {0, 1}, fd2[2] = {0, 1};
    int pos = -1;
    pid_t pid;
    for (int i = 0; i < count; i++) {
        if (swap_dir(list_parts[i]) == 1) {
            continue;
        }
        if (i != (count - 1)) {
            pipe(fd2);
        }
        pid = fork();
        if (pid == 0) {
            if (i != 0) {
                dup2(fd1[0], 0);
                close(fd1[0]);
                close(fd1[1]);
            }
            if (i != (count - 1)) {
                dup2(fd2[1], 1);
                close(fd2[0]);
                close(fd2[1]);
            }
            if (implementation(list_parts[i]) == -1)
                return -1;
            return 1;
        } else if (pid > 0) {
            if (i != 0) {
                close(fd1[0]);
                close(fd1[1]);
            }
            if (i != (count - 1)) {
                fd1[0] = fd2[0];
                fd1[1] = fd2[1];
            }
        }
    }
    if (count > 1) {
        close(fd1[0]);
        close(fd1[1]);
    }
    return 0;
}

int conv2(char ***list_parts, int count) {
    int pos = -1;
    pid_t pid;
    for (int i = 0; i < count; i++) {
        if (swap_dir(list_parts[i]) == 1) {
                continue;
        }
        pid = fork();
        pos = search(list_parts[i], "&");
        if (pos != -1) {
            list_parts[i][pos] = NULL;
        }
        if (pid == 0) {
            if (implementation(list_parts[i]) == -1)
                return -1;
            return 1;
        }
    }
    return 0;
}

int additional_hall(char **list) {
    int count = 0, conv = 0, ans = 0;
    char ***list_parts = great_divider(list, &count, "&&");
    if (count == 1) {
        for (int i = 0; i < count; i++) {
            free(list_parts[i]);
        }
        free(list_parts);
        list_parts = great_divider(list, &count, "|");
        conv = 1;
    }
    if (conv == 1) {
        ans = conv1(list_parts, count);
    } else {
        ans = conv2(list_parts, count);
    }
    for (int i = 0; i < count; i++) {
        free(list_parts[i]);
    }
    free(list_parts);
    return ans;
}

int major_hall(char **list) {
    list = get_list();
    int check = 0;
    while (cmp_exit(list[0])) {
        if (strcmp(list[0], "\0") != 0) {
            check = additional_hall(list);
        }
        delete_list(list);
        if (check == -1) {
            return 1;
        }
        list = get_list();
        if (check == 1) {
            return 0;
        }
    }
    delete_list(list);
    return 0;
}

int main() {
    char **list = NULL;
    signal(SIGINT, handler);
    int check = major_hall(list);
    return check;
}
