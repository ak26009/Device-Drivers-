#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "my_header.h"
#include <unistd.h>  

int main() {
    int fd = open("/dev/mydev1", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        return -1;
    }

    // Turn LED ON
    ioctl(fd, LED_ON);

    // Turn LED OFF
    ioctl(fd, LED_OFF);

    // Get status
    int st;
    ioctl(fd, GET_STATUS, &st);
    printf("LED Status = %d\n", st);

    // Set blink rate
    int rate = 1000; // milliseconds
    ioctl(fd, SET_BLINK, &rate);

    close(fd);
    return 0;
}
