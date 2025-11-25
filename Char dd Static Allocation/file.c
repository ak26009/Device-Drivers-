#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#define MAJOR_NUM 400

static dev_t dev=MKDEV(MAJOR_NUM,0);
static struct cdev my_cdev;
static char k_b[1024];

static int my_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "my_dev:Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "my_dev:Device closed\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count,loff_t *f_pos)
{
    	printk(KERN_INFO "my_dev:Read request\n");
    	if(copy_to_user(buf,k_b,count))
		return -EFAULT;
	return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count,loff_t *f_pos)
{
	printk(KERN_INFO "my_dev:WRITE request\n");
        if(copy_from_user(k_b,buf,count))
       		 return -EFAULT;
        return count;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.open=my_open,
	.read=my_read,
	.write=my_write,
	.release=my_release,
};

static int __init my_init(void){
        printk(KERN_ALERT "my_dev:initializing module\n");
	if(register_chrdev_region(dev,1,"my_dev")<0){
	printk(KERN_ALERT "my_dev:couldn't reg\n");
        return -1;
	}
	cdev_init(&my_cdev,&fops);
	if(cdev_add(&my_cdev,dev,1)<0){
	unregister_chrdev_region(dev,1);
	printk(KERN_ALERT "my_dev:cdev add failed\n");
	return -1;
	}
return 0;
}

static void my_exit(void){
	cdev_del(&my_cdev);
        printk(KERN_ALERT "my_dev:done\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
