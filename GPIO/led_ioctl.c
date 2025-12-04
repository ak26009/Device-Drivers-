// led_ioctl.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "led_driver"

/* ------------ IOCTL DEFINITIONS ------------ */

#define LED_IOC_MAGIC  'L'

#define LED_IOC_ON     _IO(LED_IOC_MAGIC, 0)        /* Turn LED ON  */
#define LED_IOC_OFF    _IO(LED_IOC_MAGIC, 1)        /* Turn LED OFF */
#define LED_IOC_SET    _IOW(LED_IOC_MAGIC, 2, int)  /* Set LED using int (0/1) */
#define LED_IOC_GET    _IOR(LED_IOC_MAGIC, 3, int)  /* Get LED state as int    */

/* ------------ GLOBALS ------------ */

static dev_t led_dev_number;
static struct cdev led_cdev;
static struct class *led_class;
static struct gpio_desc *led_gpio;

/* ------------ IOCTL HANDLER ------------ */

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

    default:
        return -ENOTTY; /* Command not supported */
    }

    return 0;
}

/* ------------ FILE OPERATIONS ------------ */

static const struct file_operations led_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl   = led_ioctl,
#endif
};

/* ------------ PROBE ------------ */

static int led_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("led_ioctl: probe() called\n");

    /* Get GPIO from Device Tree, initialized low */
    led_gpio = gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpio)) {
        pr_err("Failed to get GPIO from DT: %ld\n", PTR_ERR(led_gpio));
        return PTR_ERR(led_gpio);
    }

    /* Register char device region */
    ret = alloc_chrdev_region(&led_dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("alloc_chrdev_region failed: %d\n", ret);
        goto err_gpio_put;
    }

    /* class_create(new API: only takes name) */
    led_class = class_create(DEVICE_NAME);
    if (IS_ERR(led_class)) {
        ret = PTR_ERR(led_class);
        pr_err("class_create failed: %d\n", ret);
        goto err_unregister_chrdev;
    }

    /* Initialize and add cdev */
    cdev_init(&led_cdev, &led_fops);
    ret = cdev_add(&led_cdev, led_dev_number, 1);
    if (ret < 0) {
        pr_err("cdev_add failed: %d\n", ret);
        goto err_class_destroy;
    }

    /* Create /dev/led_driver node */
    if (!device_create(led_class, NULL, led_dev_number, NULL, DEVICE_NAME)) {
        pr_err("Failed to create device node\n");
        ret = -EINVAL;
        goto err_cdev_del;
    }

    pr_info("led_ioctl: created /dev/%s\n", DEVICE_NAME);
    return 0;

err_cdev_del:
    cdev_del(&led_cdev);
err_class_destroy:
    class_destroy(led_class);
err_unregister_chrdev:
    unregister_chrdev_region(led_dev_number, 1);
err_gpio_put:
    gpiod_put(led_gpio);
    return ret;
}

/* ------------ REMOVE (VOID) ------------ */

static void led_remove(struct platform_device *pdev)
{
    /* Turn LED off on remove */
    gpiod_set_value_cansleep(led_gpio, 0);
    gpiod_put(led_gpio);

    device_destroy(led_class, led_dev_number);
    cdev_del(&led_cdev);
    class_destroy(led_class);
    unregister_chrdev_region(led_dev_number, 1);

    pr_info("led_ioctl: removed\n");
}

/* ------------ DT MATCH TABLE ------------ */

static const struct of_device_id led_of_match[] = {
    { .compatible = "mycompany,rpi-gpio-led" },
    { }
};
MODULE_DEVICE_TABLE(of, led_of_match);

/* ------------ PLATFORM DRIVER ------------ */

static struct platform_driver led_platform_driver = {
    .probe  = led_probe,
    .remove = led_remove, /* void for platform_driver */
    .driver = {
        .name           = "rpi-gpio-led-driver-ioctl",
        .of_match_table = led_of_match,
    },
};

module_platform_driver(led_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("RPi GPIO LED chardev driver using ioctl only");

