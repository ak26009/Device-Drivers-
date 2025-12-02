#include <linux/timer.h>
#include<linux/module.h>
#include<linux/init.h>

static struct timer_list my_timer;

void timer_callback(struct timer_list *t)
{
    printk("Timer Running\n");
    //mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
}

static int __init my_init(void)
{
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Exit:!!!!");
    del_timer(&my_timer);
}
 
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
