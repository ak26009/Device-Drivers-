#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/mydev1", O_WRONLY);
    char msg[] = "Writer 2 data\n";
    write(fd, msg, sizeof(msg));
    close(fd);
}
