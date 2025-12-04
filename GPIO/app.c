// led_user.c  – User program to control LED using ioctl()

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* Must match the kernel driver's definitions */
#define LED_IOC_MAGIC  'L'
#define LED_IOC_ON     _IO(LED_IOC_MAGIC, 0)
#define LED_IOC_OFF    _IO(LED_IOC_MAGIC, 1)
#define LED_IOC_SET    _IOW(LED_IOC_MAGIC, 2, int)
#define LED_IOC_GET    _IOR(LED_IOC_MAGIC, 3, int)

#define DEVICE "/dev/led_driver"

int main()
{
    int fd, choice, value;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    while (1) {
        printf("\n===== LED IOCTL MENU =====\n");
        printf("1. LED ON\n");
        printf("2. LED OFF\n");
        printf("3. LED SET (0/1)\n");
        printf("4. LED GET\n");
        printf("5. EXIT\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1:
            ioctl(fd, LED_IOC_ON);
            printf("LED turned ON\n");
            break;

        case 2:
            ioctl(fd, LED_IOC_OFF);
            printf("LED turned OFF\n");
            break;

        case 3:
            printf("Enter value (0 or 1): ");
            scanf("%d", &value);
            ioctl(fd, LED_IOC_SET, &value);
            printf("LED set to %d\n", value);
            break;

        case 4:
            ioctl(fd, LED_IOC_GET, &value);
            printf("LED state = %d\n", value);
            break;

        case 5:
            close(fd);
            return 0;

        default:
            printf("Invalid choice!\n");
        }
    }

    close(fd);
    return 0;
}
