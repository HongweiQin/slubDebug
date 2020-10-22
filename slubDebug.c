#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MAX_SLABS (16)

static struct kmem_cache *my_slabs[MAX_SLABS];


static int find_empty_slot(void) {
	int i;

	for (i = 0; i < MAX_SLABS; i++)
		if (!my_slabs[i])
			return i;

	return MAX_SLABS;
}

/* a @slot */
static void allocate_mem(char *cmd) {
	int slot;
	
	sscanf(cmd, "%d", &slot);
	if (slot >= 0 && slot < MAX_SLABS) {
		void *p =
			kmem_cache_alloc(my_slabs[slot], GFP_KERNEL);

		pr_notice("Allocate from slab[%d] success, addr=0x%lx\n",
					slot, (unsigned long)p);
	}
}

/* f @slot @p */
static void free_mem(char *cmd) {
	int slot;
	unsigned long pi;
	
	sscanf(cmd, "%d %lx", &slot, &pi);
	if (slot >= 0 && slot < MAX_SLABS) {
 		void *p =
 			(void *)pi;

		pr_notice("Free to slab[%d], addr=0x%lx\n",
					slot, (unsigned long)p);
		kmem_cache_free(my_slabs[slot], p);
	}
}


static void __destroy_slab(int slot, int print) {
	if (slot >= 0 && slot < MAX_SLABS) {
		if (my_slabs[slot]) {
			kmem_cache_destroy(my_slabs[slot]);
			my_slabs[slot] = NULL;
			if (print)
				pr_notice("Destroy finished\n");
			return;
		}
	}
	if (print)
		pr_notice("Error\n");
}

/* d @slot */
static void destroy_slab(char *cmd, int print) {
	int slot;

	sscanf(cmd, "%d", &slot);

	__destroy_slab(slot, print);	
}

/* n @size */
static void allocate_new_slab(char *cmd) {
	int init_size;
	int slot;
	char slabname[64];

	sscanf(cmd, "%d", &init_size);
	slot = find_empty_slot();
	if (slot == MAX_SLABS) {
		pr_notice("no slot available\n");
		return;
	}

	sprintf(slabname, "slubDebug_%d", slot);
	my_slabs[slot] = kmem_cache_create(slabname, init_size, 0,
								SLAB_HWCACHE_ALIGN,
								NULL);
	if (my_slabs[slot]) {
		pr_notice("Created, slot=%d\n", slot);
	} else {
		pr_notice("Creat failed\n");
	}
}

static ssize_t slubDebug_write(struct file *file,
				const char __user *buffer,
				size_t count, loff_t *ppos)
{
	char usrCommand[512];
	int ret;

	ret = copy_from_user(usrCommand, buffer,count);
	switch (usrCommand[0]) {
	case 'a':
		pr_notice("%s, a\n", __func__);
		allocate_mem(&usrCommand[1]);
		break;
	case 'd':
		pr_notice("%s, d\n", __func__);
		destroy_slab(&usrCommand[1], 1);
		break;
	case 'f':
		pr_notice("%s, f\n", __func__);
		free_mem(&usrCommand[1]);
		break;
	case 'n':
		pr_notice("%s, n\n", __func__);
		allocate_new_slab(&usrCommand[1]);
		break;
	}
	return count;
}


static const struct file_operations slubDebug_proc_fops = {
  .owner = THIS_MODULE,
  .write = slubDebug_write,
};

static int __init slub_debug_module_init(void)
{
	int i;

	for (i = 0; i < MAX_SLABS; i++)
		my_slabs[i] = NULL;
	proc_create("slubDebug", 0, NULL, &slubDebug_proc_fops);
	pr_notice("slub debug loaded\n");
	return 0;
}

static void slub_debug_module_exit(void)
{
	int i;

	remove_proc_entry("slubDebug", NULL);
	for (i = 0; i < MAX_SLABS; i++)
		__destroy_slab(i, 0);
	pr_notice("slub debug removed\n");
}

module_init(slub_debug_module_init);
module_exit(slub_debug_module_exit);
MODULE_AUTHOR("Hongwei Qin <glqhw@hust.edu.cn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A kernel module for learning slub debugging.");

