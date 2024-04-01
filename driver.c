#include <linux/init.h>
#include <linux/module.h>

#define SUCCESS 0
#define FAILURE -1

static int __init my_init(void) {
	pr_info("hello\n");
	return SUCCESS;
}

static void __exit my_exit(void) {
	pr_info("bye\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");

