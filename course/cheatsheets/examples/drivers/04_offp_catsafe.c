// drivers.md §6d — position kept in *offp (per open file): cat stops, every open() starts at 0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SIZE 512
static char mem[SIZE];
static size_t len;                     /* how many bytes are valid */

static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp)
{
	size_t pos = *offp;                /* this open file's position */
	size_t n;

	if (pos >= len)
		return 0;                      /* EOF for THIS open file */
	n = len - pos;
	if (count < n)
		n = count;
	if (copy_to_user(buf, mem + pos, n))
		return -EFAULT;
	*offp += n;                        /* the kernel keeps it for next read */
	return n;
}

static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp)
{
	size_t pos = *offp;
	size_t n;

	if (pos >= SIZE)
		return -ENOSPC;
	n = SIZE - pos;
	if (count < n)
		n = count;
	if (copy_from_user(mem + pos, buf, n))
		return -EFAULT;
	*offp += n;
	if (*offp > len)
		len = *offp;
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
	len = 0;
	printk(KERN_INFO "primer1: loaded (offp)\n");
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
MODULE_DESCRIPTION("offp example");
