#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 5002

int my_isalnum(char ch) {
    return ch != ' ' && ch != '\t' && ch != '\n';
}

void reverse(char *string, int fd[2]) {
    char **words = calloc(BUF_SIZE, sizeof(char*));
    char **marks = calloc(BUF_SIZE, sizeof(char*));

    for (int i = 0; i < BUF_SIZE; ++i) {
        words[i] = calloc(BUF_SIZE, sizeof(char));
        marks[i] = calloc(BUF_SIZE, sizeof(char));
    }

    int words_count = 0;
    int marks_count = 0;
    for (int i = 0; i < strlen(string); ++i) {
        char buf[BUF_SIZE];
        for (int j = i; j < strlen(string); ++j) {
            buf[j - i] = string[j];
            if (my_isalnum(string[i]) != my_isalnum(string[j])) {
                buf[j - i] = '\0';

                if (my_isalnum(string[i])) {
                    strcpy(words[words_count++], buf);
                } else {
                    strcpy(marks[marks_count++], buf);
                }

                buf[0] = '\0';
                i = j - 1;
                break;
            } else if (j == strlen(string) - 1) {
                buf[j - i + 1] = '\0';

                if (my_isalnum(string[i])) {
                    strcpy(words[words_count++], buf);
                } else {
                    strcpy(marks[marks_count++], buf);
                }

                buf[0] = '\0';
                i = j;
                break;
            }
        }
    }

    char *result = calloc(BUF_SIZE, sizeof(char));
    while (words_count > -1 || marks_count > -1) {
        --words_count;
        --marks_count;
        if (my_isalnum(string[0])) {
            if (words_count > -1) {
                strcat(result, words[words_count]);
            }
            if (marks_count > -1) {
                strcat(result, marks[marks_count]);
            }
        } else {
            if (marks_count > -1) {
                strcat(result, marks[marks_count]);
            }
            if (words_count > -1) {
                strcat(result, words[words_count]);
            }
        }
    }

    close(fd[0]);
    printf("%s\n", result);
//    write(fd[1], result, BUF_SIZE);

    for (int i = 0; i < BUF_SIZE; ++i) {
        free(words[i]);
        free(marks[i]);
    }

    free(words);
    free(marks);
    free(result);
}

int main(int argc, char **argv) {
    int fd[2];
    char *str = calloc(10000, sizeof(char));

    int file = open("/mnt/c/Users/Vlad/CLionProjects/OS_IHW1/tests/test5.txt", O_RDONLY);
    read(file, str, 5000);

    pipe(fd);
    reverse(str, fd);
    free(str);
    return 0;
}
