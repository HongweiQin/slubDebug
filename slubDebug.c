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

/* r @pointer @size
 * size can be 8, 16, 32, 64 bits
 */
static void debug_read(char *cmd) {
	unsigned long pi;
	unsigned long size;
	
	sscanf(cmd, "%lx %lu", &pi, &size);

	switch (size) {
	case 8:
		do {
			u8 sd;
			u8 *p = (u8 *)pi;

			sd = *p;
			pr_notice("read finished, result=%d =0x%x\n", sd, sd);
		} while (0);
		break;
	case 16:
		do {
			u16 sd;
			u16 *p = (u16 *)pi;

			sd = *p;
			pr_notice("read finished, result=%d =0x%x\n", sd, sd);
		} while (0);
		break;
	case 32:
		do {
			u32 sd;
			u32 *p = (u32 *)pi;

			sd = *p;
			pr_notice("read finished, result=%d =0x%x\n", sd, sd);
		} while (0);
		break;
	case 64:
		do {
			u64 sd;
			u64 *p = (u64 *)pi;

			sd = *p;
			pr_notice("read finished, result=%lld =0x%llx\n", sd, sd);
		} while (0);
		break;
	default:
		pr_notice("Not supported\n");
		break;
	}
}


/* w @pointer @size @data
 * size can be 8, 16, 32, 64 bits
 */
static void debug_write(char *cmd) {
	unsigned long pi;
	unsigned long size, data;
	
	sscanf(cmd, "%lx %lu %lu", &pi, &size, &data);

	switch (size) {
	case 8:
		do {
			u8 sd = (u8)data;
			u8 *p = (u8 *)pi;
			
			*p = sd;
			pr_notice("write (%d) to (%p) finished\n", sd, p);
		} while (0);
		break;
	case 16:
		do {
			u16 sd = (u16)data;
			u16 *p = (u16 *)pi;

			*p = sd;
			pr_notice("write (%d) to (%p) finished\n", sd, p);
		} while (0);
		break;
	case 32:
		do {
			u32 sd = (u32)data;
			u32 *p = (u32 *)pi;

			*p = sd;
			pr_notice("write (%d) to (%p) finished\n", sd, p);
		} while (0);
		break;
	case 64:
		do {
			u64 sd = (u64)data;
			u64 *p = (u64 *)pi;

			*p = sd;
			pr_notice("write (%lld) to (%p) finished\n", sd, p);
		} while (0);
		break;
	default:
		pr_notice("Not supported\n");
		break;
	}
}


/* m @pointer @size @data
 * size can be 8, 16, 32, 64 bits
 */
static void debug_write_memset(char *cmd) {
	unsigned long pi;
	unsigned long size, data;
	
	sscanf(cmd, "%lx %lu %lu", &pi, &size, &data);

	memset((void *)pi, data, size);
	pr_notice("memset (%d) to (%p) size(%d) finished\n",
					(int)data, (void *)pi, (int) size);
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

/* n @flags @size
 * @size is in bytes
 * @flags can be the combination of the follows:
 * f: sanity_checks
 * z: red_zone
 * p: poison
 */
static void allocate_new_slab(char *cmd) {
	int init_size;
	int slot;
	char slabname[64];
	slab_flags_t flags = 0;

	cmd++;
	while (*cmd != ' ') {
		if (*cmd == 'f') {
			flags |= SLAB_CONSISTENCY_CHECKS;
		} else if (*cmd == 'z') {
			flags |= SLAB_RED_ZONE;
		} else if (*cmd == 'p') {
			flags |= SLAB_POISON;
		} else if (*cmd == 't') {
			flags |= SLAB_TRACE;
		} else if (*cmd == 'u') {
			flags |= SLAB_STORE_USER;
		} else {
			break;
		}
		cmd++;
	}

	sscanf(cmd, "%d", &init_size);
	slot = find_empty_slot();
	if (slot == MAX_SLABS) {
		pr_notice("no slot available\n");
		return;
	}

	sprintf(slabname, "slubDebug_%d", slot);
	my_slabs[slot] = kmem_cache_create(slabname, init_size, 0,
								SLAB_HWCACHE_ALIGN | SLAB_PANIC | flags,
								NULL);
	if (my_slabs[slot]) {
		pr_notice("Created, slot=%d flags=0x%x\n", slot, flags);
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
	case 'm':
		pr_notice("%s, m\n", __func__);
		debug_write_memset(&usrCommand[1]);
		break;
	case 'n':
		pr_notice("%s, n\n", __func__);
		allocate_new_slab(&usrCommand[1]);
		break;
	case 'r':
		pr_notice("%s, r\n", __func__);
		debug_read(&usrCommand[1]);
		break;
	case 'w':
		pr_notice("%s, w\n", __func__);
		debug_write(&usrCommand[1]);
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

