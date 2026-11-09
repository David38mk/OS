// drivers.md §6b — write until full, then a flag flips and reads drain everything, then reset
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SIZE 512
static char mem[SIZE];
static int r, w, flag;                 /* flag 0 = filling, 1 = draining */

static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp)
{
	size_t n;

	if (flag == 0)
		return 0;                      /* nothing to read until full */
	n = w - r;
	if (count < n)
		n = count;
	if (copy_to_user(buf, mem + r, n))
		return -EFAULT;
	r += n;
	if (r == w)                        /* drained: back to the start */
		r = w = flag = 0;
	return n;
}

static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp)
{
	size_t n = SIZE - w;

	if (flag == 1)
		return -ENOSPC;                /* draining: no writes now */
	if (count < n)
		n = count;
	if (copy_from_user(mem + w, buf, n))
		return -EFAULT;
	w += n;
	if (w == SIZE)                     /* just became full */
		flag = 1;
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
	r = w = flag = 0;
	printk(KERN_INFO "primer1: loaded (drain flag)\n");
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
MODULE_DESCRIPTION("Drain flag example");
