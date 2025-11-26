#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev.h>

static dev_t dev;
static struct cdev my_cdev;
static char kbuf[1024];
static struct class *my_class;

static int my_open(struct inode *inode, struct file *file)
{
	pr_info("OPEn\n");
	return 0;
}

static ssize_t my_read(struct file *file, char __user *ubuf, size_t len, loff_t *off)
{
	copy_to_user(ubuf,kbuf,len);
	return len;
}
static ssize_t my_write(struct file *file, const char __user *ubuf, size_t len, loff_t *off)
{
	copy_from_user(kbuf,ubuf,len);
	return len;
}

static int my_release(struct inode *inode, struct file *file)
{
	pr_info("close\n");
	return 0;
}

static struct file_operations fops=
{
	.owner=THIS_MODULE,
	.open=my_open,
	.read=my_read,
	.write=my_write,
	.release=my_release,
};

static int __init my_init(void)
{
	alloc_chrdev_region(&dev,0,1,"mychar");
	my_class = class_create("my_class");
	device_create(my_class,NULL,dev,NULL,"mychar");
	cdev_init(&my_cdev,&fops);
	cdev_add(&my_cdev,dev,1);
	return 0;
}

static void my_exit(void)
{
	cdev_del(&my_cdev);
	device_destroy(my_class,dev);
	class_destroy(my_class);
	unregister_chrdev_region(dev,1);	
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
