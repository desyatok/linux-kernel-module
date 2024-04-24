#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include "library.h"
#include "driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("desyatok");
MODULE_DESCRIPTION("A sample driver");

#define SUCCESS 0
#define DEVICE_NAME "myrandom"

static int major;
enum
{
	DEV_NOT_USED = 0,
	DEV_EXCLUSIVE_OPEN = 1
};
static struct class *cls;

static generator *mygen = NULL;

static atomic_t already_open = ATOMIC_INIT(DEV_NOT_USED);

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.write = device_write,
	.read = device_read,
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
	generator_destroy();	
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

// буфер должен передаваться в следующем формате k c a_0 .. a_k-1 x_0 .. x_k-1 без пробелов (!)
static ssize_t device_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset)
{
	if (mygen != NULL)
	
	{
		generator_destroy();
	}
	int err = 0;
	ssize_t bytes_written = 0;
	uint8_t byte = 0;
	mygen = kmalloc(sizeof(generator), GFP_KERNEL);
	if (mygen == NULL) return -ENOMEM;

	err = get_user(mygen->k, buffer++);
	if (err != 0) 
	{
		kfree(mygen);
		mygen = NULL;
		return -EFAULT;
	}
	bytes_written++;

	pr_info("k written is %d", mygen->k);

	err = get_user(byte, buffer++);
	if (err != 0) 
	{
		kfree(mygen);
		mygen = NULL;
		return -EFAULT;
	}
	mygen->c = uint8_to_ff(byte);
	if (mygen->c == NULL)
	{
		kfree(mygen);
		mygen = NULL;
		return -ENOMEM;
	}
	bytes_written++;
	pr_info("c written is %d", byte);

	mygen->a = kmalloc(mygen->k * sizeof(FieldMember *), GFP_KERNEL);
	mygen->x = kmalloc(mygen->k * sizeof(FieldMember *), GFP_KERNEL);
	if (mygen->a == NULL || mygen->x == NULL)
	{
		kfree(mygen);
		mygen = NULL;
		return -ENOMEM;
	}

	for (size_t i = 0; i < mygen->k; ++i)
	{
		err = get_user(byte, buffer++);
		if (err != 0)
		{
			kfree(mygen);
			mygen = NULL;
			return -EFAULT;
		}
		bytes_written++;

		pr_info("a[%lu] is %d", i, byte);

		mygen->a[i] = uint8_to_ff(byte);
		if (mygen->a[i] == NULL)
		{
			kfree(mygen);
			mygen = NULL;
			return -ENOMEM;
		}

	}

	for (size_t i = 0; i < mygen->k; ++i)
	{
		err = get_user(byte, buffer++);
		if (err != 0)
		{
			kfree(mygen);
			mygen = NULL;
			return -EFAULT;
		}
		bytes_written++;

		pr_info("x[%lu] is %d", i, byte);

		mygen->x[i] = uint8_to_ff(byte);
		if (mygen->x[i] == NULL)
		{
			kfree(mygen);
			mygen = NULL;
			return -ENOMEM;
		}

	}

	pr_info("%ld", bytes_written);
	return bytes_written + 1;
}

static void generator_info(void)
{
	pr_info("generator values are:");
	for (size_t i = 0; i < mygen->k; ++i)
	{
		pr_info("x[%lu] is %d", i, ff_to_uint8(mygen->x[i]));
	}
}

// количество запрашиваемых псевдослучайных чисел равно count (????????)
static ssize_t device_read(struct file *file, char __user *buffer, size_t count, loff_t *offset)
{
	if (mygen == NULL) return -EPERM;
	if (count == 0) return -EINVAL;
	ssize_t bytes_read = 0;
	pr_info("%lu bytes to generate", count);
	int err = 0;
	for (size_t j = 0; j < count; ++j)
	{
		uint8_t byte = get_pseudorandom_byte();

		err = put_user(byte, buffer++);
		if (err != 0) return -EFAULT;
		bytes_read++;
		generator_info();
	}
	
	return bytes_read;
}

static uint8_t get_pseudorandom_byte(void)
{
	uint8_t byte = 0;
	FieldMember *sum = uint8_to_ff(0);
	for (size_t i = 0; i < mygen->k; ++i)
	{
		FieldMember *tmp = sum;
		FieldMember *product = ffMul(mygen->a[i], mygen->x[i]);
		sum = ffAdd(sum, product);
		freeFieldMember(tmp, 0);
		freeFieldMember(product, 0);
	}
	FieldMember *tmp = sum;
	sum = ffAdd(sum, mygen->c);	
	freeFieldMember(tmp, 0);
	
	byte = ff_to_uint8(sum);
	generator_shift_left(sum);
	return byte;
}

static void generator_shift_left(FieldMember *new_elem)
{
	freeFieldMember(mygen->x[0], 1);
	for (size_t i = 0; i < mygen->k - 1; ++i)
	{
		mygen->x[i] = mygen->x[i + 1];
	}
	mygen->x[mygen->k - 1] = new_elem;
}

static void generator_destroy(void)
{
	if (mygen == NULL) return;
	for (size_t i = 0; i < mygen->k; ++i)
	{
		freeFieldMember(mygen->a[i], 1);
		freeFieldMember(mygen->x[i], 1);
	}
	freeFieldMember(mygen->c, 1);
	kfree(mygen);
}

module_init(my_init);
module_exit(my_exit);

