#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> //kfree()
#include <linux/wait.h>

#define DEV_NAME "myworkque"

static struct cdev my_cdev;
static dev_t dev;
static int data = 0;  
static wait_queue_head_t *wq; // dynamic wait queue
static struct class *my_class;

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "myworkque: read() called, waiting for data...\n");
    wait_event_interruptible(*wq, data != 0);
    printk(KERN_INFO "myworkque: data available, returning to user.\n");
    data = 0;
    return 0;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "myworkque: write() called, waking readers\n");
    data = 1;
    wake_up_interruptible(wq);
    return count;
}

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("myworkque: device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("myworkque: device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
};

static int __init my_init(void)
{
    int ret;

    // allocate dynamic major
    ret = alloc_chrdev_region(&dev, 0, 1, DEV_NAME);
    if (ret < 0) {
        pr_err("myworkque: Unable to allocate major and minor\n");
        return ret;
    }

    // allocate memory for wait queue
    wq = kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
    if (!wq) {
        unregister_chrdev_region(dev, 1);
        return -ENOMEM;
    }

    init_waitqueue_head(wq); // initialize wait queue

    cdev_init(&my_cdev, &fops);

    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0) {
        pr_err("myworkque: Device can't be created\n");
        kfree(wq);
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    my_class = class_create(DEV_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        kfree(wq);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(my_class);
    }

    device_create(my_class, NULL, dev, NULL, DEV_NAME);
    pr_info("myworkque: Major number: %d, Minor number: %d\n", MAJOR(dev), MINOR(dev));
    return 0;
}

static void __exit my_exit(void)
{   
    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    kfree(wq);
    unregister_chrdev_region(dev, 1);
    pr_info("myworkque: unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
