#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include "library.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("desyatok");
MODULE_DESCRIPTION("A sample driver");

#define SUCCESS 0
#define DEVICE_NAME "myrandom"
#define KEY_LEN 256

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
//static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static int major;

static enum
{
	DEV_NOT_USED = 0,
	DEV_EXCLUSIVE_OPEN = 1,
};

static atomic_t already_open = ATOMIC_INIT(DEV_NOT_USED);

static struct class *cls;

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
//	.write = device_write,
};

static int __init my_init(void) 
{
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) 
	{
		pr_alert("Registering a device failed with %d\n", major);
		return major;
	}

	pr_info("Device was assigned major number %d\n", major);

	cls = class_create(DEVICE_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

	pr_info("Device was created on /dev/%s\n", DEVICE_NAME);

	return SUCCESS;
}

static void __exit my_exit(void) 
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, DEVICE_NAME);
	pr_info("Device was successfully destroyed and unregistered\n");
}

static int device_open(struct inode *inode, struct file *file) 
{
	if (atomic_cmpxchg(&already_open, DEV_NOT_USED, DEV_EXCLUSIVE_OPEN)) 
	{
		return -EBUSY;
	}

	pr_info("Successfully opened a device\n");
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) 
{
	atomic_set(&already_open, DEV_NOT_USED);
	module_put(THIS_MODULE);
	pr_info("Successfully closed a device\n");
	return SUCCESS;
}

module_init(my_init);
module_exit(my_exit);

