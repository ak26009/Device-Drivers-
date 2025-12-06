#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include "io.h"

int main() {
    int fd = open("/dev/led_driver", O_RDWR);
    int brightness=0;
    while(1)
    {
        
        // printf("Enter brightness (0-99): ");
        // scanf("%d", &brightness);
        // int time=brightness*9999;
        while(brightness<100) {
            ioctl(fd, LED_IOC_BRIGHTNESS, &brightness);
            brightness++;
            usleep(10000);
        }
        while(brightness>0)
        {
            ioctl(fd,LED_IOC_BRIGHTNESS,&brightness);
            brightness--;
            usleep(10000);
        }
    }

    return 0;
}



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






/dts-v1/;
/plugin/;
/{

                compatible="brcm,bcm2712";

                fragment@0{
                        target-path="/";

                        __overlay__{
                        led_test{
                                compatible="mycompany,rpi-gpio-led";

                                /*GPIO26 on RP1 GPIO header,active-high(0)*/
                                led-gpios=<&rp1_gpio 26 0>;

                                status = "okay";
                        };
                };
        };
};




#define MAGIC_NUM 'L'

#define LED_IOC_ON _IO(MAGIC_NUM,0)
#define LED_IOC_OFF _IO(MAGIC_NUM,1)
#define LED_IOC_SET _IOW(MAGIC_NUM,2,int)
#define LED_IOC_GET _IOR(MAGIC_NUM,3,int)
#define LED_IOC_BRIGHTNESS _IOW(MAGIC_NUM,4,int)



