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
    int choice;

    fd = open("/dev/Atharva", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    while (1) {
        printf("\n----- MENU -----\n");
        printf("1. Clear Buffer\n");
        printf("2. Write Data\n");
        printf("3. Read Data\n");
        printf("4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();  // clear leftover newline from buffer

        switch (choice) {

        case 1:
            ioctl(fd, CLEAR_BUFF);
            printf("Kernel buffer cleared.\n");
            break;

        case 2:
            printf("Enter data: ");
            getchar();
            fgets(ubuff, sizeof(ubuff), stdin);
            //len = strlen(ubuff);
            
            ioctl(fd, WRITE_BUFF, len);
            write(fd, ubuff, len);

            printf("Data written.\n");
            break;

        case 3:
            ioctl(fd, READ_BUFF, &len);
            lseek(fd, 0, SEEK_SET);
            read(fd, ubuff, len);
            ubuff[len] = '\0';

            printf("Data read: %s\n", ubuff);
            break;

        case 4:
            close(fd);
            return 0;

        default:
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
