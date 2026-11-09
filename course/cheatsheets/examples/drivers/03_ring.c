// drivers.md §6c — circular buffer: indices wrap around, a counter tells full from empty
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define SIZE 64
static char ring[SIZE];
static int r, w, cnt;                  /* cnt = bytes currently stored */

static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp)
{
	size_t n = count < cnt ? count : cnt;
	size_t first = SIZE - r;           /* bytes until the end of the array */

	if (n == 0)
		return 0;
	if (first > n)
		first = n;
	if (copy_to_user(buf, ring + r, first) ||
	    copy_to_user(buf + first, ring, n - first))
		return -EFAULT;                /* 2nd copy has size 0 if no wrap */
	r = (r + n) % SIZE;
	cnt -= n;
	return n;
}

static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp)
{
	size_t space = SIZE - cnt;
	size_t n = count < space ? count : space;
	size_t first = SIZE - w;

	if (space == 0)
		return -ENOSPC;
	if (first > n)
		first = n;
	if (copy_from_user(ring + w, buf, first) ||
	    copy_from_user(ring, buf + first, n - first))
		return -EFAULT;
	w = (w + n) % SIZE;
	cnt += n;
	return n;
}

static const struct file_operations primer_fops = {
	.owner = THIS_MODULE,
	.read  = primer_read,
	.write = primer_write,
};

static int __init primer_init(void)
{
	int ret = register_chrdev(60, "primer1", &primer_fops);
	if (ret < 0)
		return ret;
	r = w = cnt = 0;
	printk(KERN_INFO "primer1: loaded (ring)\n");
	return 0;
}

static void __exit primer_exit(void)
{
	unregister_chrdev(60, "primer1");
	printk(KERN_INFO "primer1: unloaded\n");
}

module_init(primer_init);
module_exit(primer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS course");
MODULE_DESCRIPTION("Ring buffer example");
