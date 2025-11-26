static int __init mychar_init(void)
{
    alloc_chrdev_region(&dev_num, 0, 1, "mychar");

    mychar_class = class_create(THIS_MODULE, "mychar_class");

    device_create(mychar_class, NULL, dev_num, NULL, "mychar");

    cdev_init(&mychar_cdev, &fops);

    cdev_add(&mychar_cdev, dev_num, 1);

    return 0;
}
