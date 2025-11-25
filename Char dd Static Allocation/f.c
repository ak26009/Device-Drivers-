#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;
    char write_buf[] = "Hello from user space!";
    char read_buf[100];

    /* Open the device file */
    fd =open("/dev/my_dev", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* Write data */
    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }
    printf("Wrote to device: %s\n", write_buf);
    if (read(fd, read_buf, sizeof(read_buf)) < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("Read from device: %s\n", read_buf);

    close(fd);
    return 0;
}
