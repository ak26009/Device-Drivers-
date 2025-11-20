#include<linux/init.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");

static int __init my_init(void){
	printk(KERN_ALERT "HI\n");
	return 0;
}

static void my_exit(void){
	printk(KERN_ALERT "bye\n");
}

module_init(my_init);
module_exit(my_exit);
