#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/cdev.h>

#define DEVICE_NAME "simplewq"
#define MAJOR 230

static dev_t dev = MKDEV(MAJOR, 0);
static struct cdev my_cdev;
static int data_available = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "simplewq: read() called, waiting for data...\n");
    wait_event_interruptible(wq, data_available != 0);
    printk(KERN_INFO "simplewq: data available, returning to user.\n");
    data_available = 0;
    return 0;
}
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "simplewq: write() called, waking readers\n");
    data_available = 1;
    wake_up_interruptible(&wq);
    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
    int ret;
    ret = register_chrdev_region(dev, 1, DEVICE_NAME);
    if(ret < 0){
        pr_err("simplewq: cannot register major %d\n", MAJOR);
        return ret;
    }

    cdev_init(&my_cdev,&fops);
    ret = cdev_add(&my_cdev,dev,1);
    if(ret < 0){
        pr_err("simplewq: cdev add failed!\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }
    pr_info("simplewq: loaded. Major=%d Minor=0\n",MAJOR);
    return 0;
}

static void __exit my_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("simplewq: unloaded.\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brahmesh");
MODULE_DESCRIPTION("Simple static wait queue example");
