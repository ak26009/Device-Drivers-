#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/mydev0", O_WRONLY);
    char msg[] = "Hello from Writer 2\n";
    write(fd, msg, sizeof(msg));
    close(fd);
}
