#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/my_dev"

int main() {
    int fd, choice;
    char buf[100];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Write\n");
        printf("2. Read\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1:
            printf("Enter text: ");
            scanf(" %[^\n]", buf);
            write(fd, buf, strlen(buf));
            printf("Written.\n");
            break;

        case 2:
            memset(buf, 0, sizeof(buf));
            read(fd, buf, sizeof(buf));
            printf("Read: %s\n", buf);
            break;

        case 3:
            close(fd);
            printf("Goodbye!\n");
            return 0;

        default:
            printf("Invalid choice!\n");
        }
    }

    return 0;
}
