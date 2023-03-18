#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 5002
#define PIPE1 "/home/vlad/OS/IHW1/pipe1_2.fifo"
#define PIPE2 "/home/vlad/OS/IHW1/pipe2_1.fifo"

void read_from_file(const char *filename) {
    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1) {
        exit(-1);
    }

    char buf[BUF_SIZE] = {0};
    size_t end_of_string;
    if ((end_of_string = read(fd, buf, BUF_SIZE - 2)) == -1) {
        exit(-1);
    }
    buf[end_of_string] = '\0';

    close(fd);
    int pipe = open(PIPE1, O_WRONLY);
    write(pipe, buf, BUF_SIZE);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Incorrect count of command line parameters!\n");
        exit(-1);
    }

    (void) umask(0);
    mknod(PIPE1, S_IFIFO | 0666, 0);
    mknod(PIPE2, S_IFIFO | 0666, 0);

    //  first logic block
    read_from_file(argv[1]);

    // third logic block
    int pipe = open(PIPE2, O_RDONLY);
    char buf[BUF_SIZE] = {0};
    read(pipe, buf, BUF_SIZE - 2);

    int file_output = open(argv[2], O_CREAT | O_WRONLY);
    write(file_output, buf, strlen(buf));
    printf("%s", buf);
    close(file_output);

    unlink(PIPE1);
    unlink(PIPE2);
    return 0;
}