#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "my_header.h"

#define DEVICE_NAME "mydev1"

static dev_t dev;
static struct cdev my_cdev;
static struct class *cls;

static int led_status = 0;
static int blink_rate = 0;

// ---- IOCTL function ----
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case LED_ON:
            led_status = 1;
            printk(KERN_INFO "LED turned ON\n");
            break;

        case LED_OFF:
            led_status = 0;
            printk(KERN_INFO "LED turned OFF\n");
            break;

        case GET_STATUS:
            if (copy_to_user((int __user *)arg, &led_status, sizeof(led_status)))
                return -EFAULT;
            printk(KERN_INFO "Status sent to user: %d\n", led_status);
            break;

        case SET_BLINK:
            if (copy_from_user(&blink_rate, (int __user *)arg, sizeof(blink_rate)))
                return -EFAULT;
            printk(KERN_INFO "Blink rate set to %d ms\n", blink_rate);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

// ---- Open and Release ----
static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// ---- File operations ----
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .unlocked_ioctl = my_ioctl,
};

// ---- Module Init ----
static int __init mydriver_init(void)
{
    int ret;

    // Allocate device number
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret) {
        printk(KERN_ERR "alloc_chrdev_region failed\n");
        return ret;
    }

    printk(KERN_INFO "Major=%d Minor=%d\n", MAJOR(dev), MINOR(dev));

    // Initialize cdev
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev, 1);
    if (ret) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "cdev_add failed\n");
        return ret;
    }

    // Create class & device (for /dev/mydev1)
    cls = class_create("myclass1");
    if (IS_ERR(cls)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "class_create failed\n");
        return PTR_ERR(cls);
    }

    if (IS_ERR(device_create(cls, NULL, dev, NULL, DEVICE_NAME))) {
        class_destroy(cls);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "device_create failed\n");
        return -1;
    }

    printk(KERN_INFO "mydriver loaded successfully\n");
    return 0;
}

// ---- Module Exit ----
static void __exit mydriver_exit(void)
{
    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "mydriver unloaded\n");
}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple ioctl char driver example");
