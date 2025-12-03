#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    char buf[128];
    int fd = open("/dev/mydev2", O_RDONLY);

    printf("Reader waiting for data (or timeout)…\n");
    int r = read(fd, buf, sizeof(buf));
    //buf[r] = 0;
    printf("Reader got: %s\n", buf);

    close(fd);
}
