#include<linux/module.h>
#include<linux/fs.h>
#include<linux/ioctl.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
#include"myhd.h"

static dev_t dev;
static struct class *my_class;
static struct cdev my_cdev;

static int led_s;
static int value;

static long myioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  switch(cmd){
    case ON:
      led_s = 1;
      printk(KERN_INFO"LED_ON\n");
      break;
    
    case OFF:
      led_s = 0;
      printk(KERN_INFO"LED_OFF\n");
      break;
    
    case STATUS:
    printk(KERN_INFO "LED_S=%d",led_s);
      if(copy_to_user((int __user*)arg,&led_s,sizeof(led_s)))
        return -EFAULT;
      printk(KERN_INFO"Status send = %d\n",led_s);
      break;
      
    case SET_VALUE:
    if(copy_from_user(&value,(int __user*)arg,sizeof(value)))
        return -EFAULT;
      printk(KERN_INFO"Value set to %d\n",value);
      break;
  }
  return 0;
}
static int myopen(struct inode *inode, struct file *file)
{
  printk(KERN_INFO"FILE OPEN\n");
  return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
  printk(KERN_INFO"FILE CLOSE\n");
  return 0;
}

static struct file_operations ops=
{
  .owner=THIS_MODULE,
  .open=myopen,
  .release=myclose,
  .unlocked_ioctl=myioctl,
};

static int __init myinit(void)
{
  alloc_chrdev_region(&dev,0,1,"Atharva");
  my_class = class_create("Personal");
  device_create(my_class,NULL,dev,NULL,"Atharva");
  cdev_init(&my_cdev,&ops);
  cdev_add(&my_cdev,dev,1);
  printk(KERN_INFO"OPEN\n");
  return 0;
}

static void myexit(void)
{
  cdev_del(&my_cdev);
  device_destroy(my_class,dev);
  class_destroy(my_class);
  unregister_chrdev_region(dev,1);
  
  printk(KERN_INFO"CLOSE\n");
}

module_init(myinit);
module_exit(myexit);
MODULE_LICENSE("GPL");
