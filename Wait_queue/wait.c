#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/sched.h>  // for TASK_INTERRUPTIBLE

dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;
static struct task_struct *my_thread;

DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);

uint32_t read_count = 0;

// Function prototypes
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static int my_thread_fn(void *unused);

// File operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
};

// Kernel thread function
static int my_thread_fn(void *data)
{
    while (!kthread_should_stop()) {
        pr_info("Thread waiting for read...\n");

        // Sleep until woken up
        wait_event_interruptible(my_wait_queue, kthread_should_stop());

        if (kthread_should_stop()) break;

        // Every wake-up from read triggers this
        pr_info("Read event received! Count: %d\n", ++read_count);
    }

    pr_info("Thread exiting.\n");
    return 0;
}

// Open function
static int my_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened.\n");
    return 0;
}

// Release function
static int my_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed.\n");
    return 0;
}

// Read function
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read called, waking up thread.\n");
    wake_up_interruptible(&my_wait_queue);  // Wake thread without flag
    return 0;
}

// Init function
static int __init my_driver_init(void)
{
    alloc_chrdev_region(&dev, 0, 1, "my_dev");
    pr_info("Major=%d Minor=%d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    my_class = class_create("my_class");
    device_create(my_class, NULL, dev, NULL, "my_device");

    my_thread = kthread_run(my_thread_fn, NULL, "MyThread");

    pr_info("Driver loaded.\n");
    return 0;
}

// Exit function
static void __exit my_driver_exit(void)
{
    if (my_thread) 
    kthread_stop(my_thread);

    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("Driver removed.\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Beginner Example");
MODULE_DESCRIPTION("Simple Linux driver with wait queue and kernel thread (no flag)");
