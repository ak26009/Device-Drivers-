#include<linux/module.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
#include<linux/kernel.h>

static int var = 10;
static char *st = "World";
static int arr[]={1,2,3};

module_param(var,int,S_IRUGO);
module_param(st,charp,S_IRUGO);
module_param(arr,&arr,S_IRUGO);

static int __init hello(void){
  pr_info("var = %d\n",var);
  pr_info("st = %s\n",st);
  return 0;
}

static void __exit ex(void){
  pr_info("Completed\n");
}

module_init(hello);
module_exit(ex);
MODULE_LICENSE("GPL");
