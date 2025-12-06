#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>

#include "io.h"
static dev_t led_dev_number;
static struct cdev led_cdev;
static struct class *led_class;
static struct gpio_desc *led_gpio;
static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;

    switch (cmd) {

    case LED_IOC_ON:
        gpiod_set_value_cansleep(led_gpio, 1);
        break;

    case LED_IOC_OFF:
        gpiod_set_value_cansleep(led_gpio, 0);
        break;

    case LED_IOC_SET:
        if (copy_from_user(&value, (int __user *)arg, sizeof(value)))
            return -EFAULT;
        gpiod_set_value_cansleep(led_gpio, value ? 1 : 0);
        break;

    case LED_IOC_GET:
        value = gpiod_get_value_cansleep(led_gpio);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        break;
    /* ------------------------------------------------------------------ */
    /* NEW BRIGHTNESS IOCTL — NO WHILE LOOP, NO THREAD, ONLY ONE PWM PULSE */
    /* ------------------------------------------------------------------ */
    case LED_IOC_BRIGHTNESS:
    {
        int duty_on, duty_off;

        if (copy_from_user(&value, (int __user *)arg, sizeof(value)))
            return -EFAULT;

        if (value < 0) value = 0;
        if (value > 100) value = 100;

        duty_on  = value;        /* percentage */
        duty_off = 100 - value;  /* percentage */

        /* ON part of PWM */
        if (duty_on > 0) {
            gpiod_set_value_cansleep(led_gpio, 1);
            udelay(duty_on * 50);     /* 50 µs per percentage */
        }
        /* OFF part of PWM */
        if (duty_off > 0) {
            gpiod_set_value_cansleep(led_gpio, 0);
            udelay(duty_off * 50);
        }
        break;
    }
    }
    return 0;
}

static const struct file_operations led_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
};

static int led_probe(struct platform_device *pdev)
{
    pr_info("led_ioctl: probe() called\n");

    led_gpio = gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);

    alloc_chrdev_region(&led_dev_number, 0, 1, "led_driver");
    led_class = class_create("led_driver");
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, led_dev_number, 1);
    device_create(led_class, NULL, led_dev_number, NULL, "led_driver");

    pr_info("led_ioctl: created /dev/%s\n", "led_driver");
    return 0;
}

static void led_remove(struct platform_device *pdev)
{
    gpiod_set_value_cansleep(led_gpio, 0);
    gpiod_put(led_gpio);
    device_destroy(led_class, led_dev_number);
    cdev_del(&led_cdev);
    class_destroy(led_class);
    unregister_chrdev_region(led_dev_number, 1);

    pr_info("led_ioctl: removed\n");
}

static const struct of_device_id led[] = {
    { .compatible = "mycompany,rpi-gpio-led" },
    { },
};
MODULE_DEVICE_TABLE(of, led);

static struct platform_driver led_platform_driver = {
    .probe  = led_probe,
    .remove = led_remove,
    .driver = {
        .name           = "rpi-gpio-led-driver-ioctl",
        .of_match_table = led,
    },
};

module_platform_driver(led_platform_driver);
MODULE_LICENSE("GPL");






#define ON _IO('l',1)
#define OFF _IO('l',2)
#define SET _IOW('l',3,int)
#define GET _IOR('l',4,int)
#define PWM _IOWR('l', 5, int)
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
int main()
{
    int value;
    int a;
    int fd=open("/dev/gpiodevice",O_RDWR);
    while(1)
    {

        printf("hello\n");
        scanf("%d",&a);
        switch(a)
        {
            case 1:
            ioctl(fd,ON);
            break;
            case 2:
            ioctl(fd,OFF);
            break;
            case 3:
            scanf("%d",&value);
            ioctl(fd,SET,&value);
            break;
            case 4:
            ioctl(fd,GET,&value);
            printf("status:%d\n",value);
            break;
            case 5:
            {
                int bright=0;
                while(1)
                {
                    while (bright<100)
                    {
                        ioctl(fd,PWM,&bright);
                        bright++;
                        usleep(10000);
                    }
                    while (bright>0)
                    {
                        ioctl(fd,PWM,&bright);
                        bright--;
                        usleep(10000);
                    }
                }
            break;
            }
            default:
            return 0;
        }
    }
}
