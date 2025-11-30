#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "myhd.h"

int main()
{
    int fd;
    int len;
    char ubuff[1024];

    fd = open("/dev/Atharva", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    
    ioctl(fd, CLEAR_BUFF);

    printf("Enter data to write: ");
    
    fgets(ubuff, sizeof(ubuff), stdin);

    len = strlen(ubuff);

    ioctl(fd, WRITE_BUFF, len);
    
    write(fd, ubuff, len);

    printf("Data written to kernel buffer.\n");

    ioctl(fd, READ_BUFF, &len);

    lseek(fd, 0, SEEK_SET); // go to beginning for read()

    read(fd, ubuff, len);
    ubuff[len] = '\0';

    printf("Data read from driver: %s\n", ubuff);

    close(fd);
    return 0;
}
