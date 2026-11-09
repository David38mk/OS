// drivers.md §6a — linear buffer: append on write, read from r up to w, EOF when r == w
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SIZE 512
static char mem[SIZE];
static int r, w;                       /* read index, write index */

static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp)
{
	size_t avail = w - r;              /* bytes written but not yet read */
	size_t n = count < avail ? count : avail;

	if (n == 0)
		return 0;                      /* r == w: end of file, cat stops */
	if (copy_to_user(buf, mem + r, n))
		return -EFAULT;
	r += n;
	return n;                          /* what we really delivered */
}

static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp)
{
	size_t space = SIZE - w;
	size_t n = count < space ? count : space;

	if (space == 0)
		return -ENOSPC;                /* full: tell the writer */
	if (copy_from_user(mem + w, buf, n))
		return -EFAULT;
	w += n;
	return n;                          /* may be < count: caller retries */
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
	r = w = 0;
	printk(KERN_INFO "primer1: loaded (linear)\n");
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
MODULE_DESCRIPTION("Linear buffer example");
