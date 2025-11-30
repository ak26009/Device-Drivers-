#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "myhd.h"

static dev_t dev;
static struct class *my_class;
static struct cdev my_cdev;

static char kbuff[1024];
static int data_len = 0;     // how many bytes valid inside kbuff

static long myioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {

    case CLEAR_BUFF:
        memset(kbuff, 0, sizeof(kbuff));
        data_len = 0;
        printk(KERN_INFO "IOCTL: Buffer Cleared\n");
        break;

    case WRITE_BUFF:
        if (arg > sizeof(kbuff))
            return -EINVAL;

        data_len = arg;
        printk(KERN_INFO "IOCTL: Ready to write %d bytes (use write())\n", data_len);
        break;

    case READ_BUFF:
        if (copy_to_user((int __user *)arg, &data_len, sizeof(data_len)))
            return -EFAULT;

        printk(KERN_INFO "IOCTL: Returned buffer length = %d\n", data_len);
        break;

    default:
        printk(KERN_WARNING "IOCTL: Unknown Command\n");
        return -EINVAL;
    }

    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    if (*off >= data_len)
        return 0;

    if (*off + len > data_len)
        len = data_len - *off;

    if (copy_to_user(buf, kbuff + *off, len))
        return -EFAULT;

    *off += len;
    return len;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    if (len > sizeof(kbuff))
        return -EINVAL;

    if (copy_from_user(kbuff, buf, len))
        return -EFAULT;

    data_len = len;
    *off = len;

    printk(KERN_INFO "WRITE: %zu bytes stored\n", len);
    return len;
}

static int myopen(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "FILE OPEN\n");
    return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "FILE CLOSE\n");
    return 0;
}

static struct file_operations ops = {
    .owner          = THIS_MODULE,
    .open           = myopen,
    .release        = myclose,
    .read           = my_read,
    .write          = my_write,
    .unlocked_ioctl = myioctl,
};

static int __init myinit(void)
{
    alloc_chrdev_region(&dev, 0, 1, "Atharva");
    my_class = class_create("Personal");
    device_create(my_class, NULL, dev, NULL, "Atharva");

    cdev_init(&my_cdev, &ops);
    cdev_add(&my_cdev, dev, 1);

    printk(KERN_INFO "Driver Loaded\n");
    return 0;
}

static void __exit myexit(void)
{
    cdev_del(&my_cdev);
    device_destroy(my_class, dev);
    class_destroy(my_class);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Driver Unloaded\n");
}

module_init(myinit);
module_exit(myexit);
MODULE_LICENSE("GPL");
