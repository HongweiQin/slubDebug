#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/crc32.h>
#include <linux/uuid.h>
#include <linux/cpumask.h>
#include <linux/lightnvm.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

static ssize_t slubDebug_write(struct file *file,
				const char __user *buffer,
				size_t count, loff_t *ppos)
{
#if 0
	char usrCommand[512];
	int ret;
	int i;
	struct qblk *qblk = debugqblk;
	spinlock_t testlock;

	ret = copy_from_user(usrCommand, buffer,count);
	//pr_notice("command:%s",usrCommand);
	switch (usrCommand[0]) {
	case 'a':
		spin_lock_init(&testlock);
		pr_notice("%s, a\n", __func__);
		debugA(9,8,7,&testlock);
		break;
	}
#endif
	return count;
}


static const struct file_operations slubDebug_proc_fops = {
  .owner = THIS_MODULE,
  .write = slubDebug_write,
};

static int __init slub_debug_module_init(void)
{
	proc_create("slubDebug", 0, NULL, &slubDebug_proc_fops);
	return 0;
}

static void slub_debug_module_exit(void)
{
	remove_proc_entry("slubDebug", NULL);
}

module_init(slub_debug_module_init);
module_exit(slub_debug_module_exit);
MODULE_AUTHOR("Hongwei Qin <glqhw@hust.edu.cn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A kernel module for learning slub debugging.");

