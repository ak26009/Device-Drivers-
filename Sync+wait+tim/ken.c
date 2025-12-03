#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/string.h>

#define DEVNAME  "mydev"
#define MAX_DEV  3
#define BUFSIZE  128

static dev_t devno;
static struct cdev cdevs[MAX_DEV];
static struct class *my_class;
static struct device *my_devices[MAX_DEV];

static char kbuf[BUFSIZE];
static int data_available = 0;

static DECLARE_WAIT_QUEUE_HEAD(wq);
static DEFINE_MUTEX(lock);

static struct timer_list my_timer;

/****************** TIMER HANDLER ******************/
static void timer_fn(struct timer_list *t)
{
    printk(KERN_INFO "Timer expired → waking reader\n");

  //  mutex_lock(&lock);
    //strcpy(kbuf, "TIMEOUT\n");
    data_available = 1;           // cause wakeup
   // mutex_unlock(&lock);

    wake_up_interruptible(&wq);
}

/****************** WRITE OPERATION ******************/
static ssize_t my_write(struct file *f, const char __user *ubuf,
                        size_t len, loff_t *off)
{
    printk(KERN_INFO "Writer called\n");

    if (len >= BUFSIZE)
        len = BUFSIZE - 1;

    mutex_lock(&lock);

    if (copy_from_user(kbuf, ubuf, len)) {
        mutex_unlock(&lock);
        return -EFAULT;
    }
    pr_info("from user:%s",kbuf);
    kbuf[len] = '\0';

    data_available = 1;
    wake_up_interruptible(&wq);

    mutex_unlock(&lock);

    return len;
}

static ssize_t my_read(struct file *f, char __user *ubuf,
                       size_t len, loff_t *off)
{
    printk(KERN_INFO "Reader waiting…\n");

    data_available = 0;

    /* restart 10-second timer */
    mod_timer(&my_timer, jiffies + 10 * HZ);

    /* sleep until writer writes OR timer expires */
    wait_event_interruptible(wq, data_available == 1);

    //mutex_lock(&lock);

    size_t outlen = strnlen(kbuf, BUFSIZE);

    if (copy_to_user(ubuf, kbuf, outlen)) {
      //  mutex_unlock(&lock);
        return -EFAULT;
    }
pr_info("%s",kbuf);
    //mutex_unlock(&lock);

    return outlen;
}

/****************** OPEN / RELEASE ******************/
static int my_open(struct inode *i, struct file *f)
{
    pr_info("file opened");
    return 0;
}

static int my_release(struct inode *i, struct file *f)
{
    return 0;
}

/****************** FILE OPS TABLE ******************/
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .read    = my_read,
    .write   = my_write,
    .open    = my_open,
    .release = my_release
};

/****************** MODULE INIT ******************/
static int __init myinit(void)
{
    int i;

    pr_info("Initializing!!!\n");
    
    /* Allocate device numbers */
    alloc_chrdev_region(&devno, 0, MAX_DEV, DEVNAME);

    /* Create the class in /sys/class/mydev */
    my_class = class_create( DEVNAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(devno, MAX_DEV);
        return PTR_ERR(my_class);
    }

    /* Initialize and add cdev + create devices */
    for (i = 0; i < MAX_DEV; i++) {

        cdev_init(&cdevs[i], &fops);
        cdev_add(&cdevs[i],
                 MKDEV(MAJOR(devno), MINOR(devno) + i),
                 1);

        /* /dev/mydev0, mydev1, mydev2 will be auto-created */
        my_devices[i] = device_create(
            my_class,
            NULL,
            MKDEV(MAJOR(devno), MINOR(devno) + i),
            NULL,
            "%s%d",
            DEVNAME,
            i
        );
    }

    timer_setup(&my_timer, timer_fn, 0);

    printk(KERN_INFO "Dynamic Char Driver Loaded\n");
    return 0;
}


/****************** MODULE EXIT ******************/
static void __exit myexit(void)
{
    int i;

    del_timer_sync(&my_timer);

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(my_class,
                       MKDEV(MAJOR(devno), MINOR(devno) + i));
        cdev_del(&cdevs[i]);
    }

    class_destroy(my_class);
    unregister_chrdev_region(devno, MAX_DEV);

    printk(KERN_INFO "Driver Removed\n");
}

module_init(myinit);
module_exit(myexit);

MODULE_LICENSE("GPL");
