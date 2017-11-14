#include<linux/module.h> 
#include<linux/kernel.h> /* printk */
#include<linux/types.h> /* dev_t */
#include<linux/kdev_t.h> /* major, minor number macro */
#include<linux/fs.h> /* register number */
#include<linux/init.h> /* module init, exit */
#include<linux/device.h> /* device class */
#include<linux/cdev.h> /* cdev, init, add */
#include <linux/uaccess.h> /* copy to user */
#include<linux/mutex.h> /* for mutex lock */

#define DEVICE_NAME "MY_First_Driver"
#define CLASS_NAME "My_Device_Class"

static dev_t number;
static struct class *device_class;
static struct cdev char_device;

static int nopen;
static char message[20];
static short msg_len;

static int my_open(struct inode *i, struct file *f);
static int my_close(struct inode *i, struct file *f);
static ssize_t my_read(struct file *f, char __user *buffer, size_t len,loff_t *off);
static ssize_t my_write(struct file *f, const char __user *buffer, size_t len,loff_t *off);

static struct file_operations my_fop =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static DEFINE_MUTEX(my_mutex_lock); /* define the mutex */

/* constructor */
static int __init my_init(void)
{
	int ret;
	struct device *dev_ret;

	printk(KERN_INFO "Welcome\n");

	/* mutex init */
	mutex_init(&my_mutex_lock);

	/* Major minor number */
	ret = alloc_chrdev_region(&number, 0, 1, DEVICE_NAME);
	if (ret < 0)
		return ret;
	printk(KERN_INFO "Major number:%d\n", MAJOR(number));
	printk(KERN_INFO "Minor number:%d\n", MINOR(number));

	/* device class */
	device_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(device_class)) {
		unregister_chrdev_region(number, 1);
		return PTR_ERR(device_class);
	}

	/* device create */
	dev_ret = device_create(device_class, NULL, number, NULL, DEVICE_NAME);
	if (IS_ERR(dev_ret)) {
		class_destroy(device_class);
		unregister_chrdev_region(number, 1);
		return PTR_ERR(dev_ret);
	}
	
	/* device init and link to vfs */
	cdev_init(&char_device, &my_fop);
	ret = cdev_add(&char_device, number, 1);
	if (ret < 0) {
		device_destroy(device_class, number);
		class_destroy(device_class);
		unregister_chrdev_region(number, 1);
		return ret;
	}
	
	return 0;
}

static void __exit my_exit(void)
{
	mutex_destroy(&my_mutex_lock);
	cdev_del(&char_device);
	device_destroy(device_class, number);
	class_destroy(device_class);
	unregister_chrdev_region(number, 1);
	printk(KERN_INFO "bye\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gaurang Rathod <gaurang.rathod@softnautics.com>");
MODULE_DESCRIPTION("First char driver");

/* my_file_functions */
static int my_open(struct inode *i, struct file *f)
{
	if (!mutex_trylock(&my_mutex_lock)) {
		printk(KERN_ALERT "Device is used by other app\n");
		return -EBUSY;
	}
	nopen++;
	printk(KERN_INFO "Driver: open %d times\n", nopen);
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: close()\n");
	mutex_unlock(&my_mutex_lock);
	return 0;
}
static ssize_t my_read(struct file *f, char __user *buffer, size_t len,loff_t *off)
{
	int error_count = 0;
	error_count = copy_to_user(buffer, message, msg_len);
	if (error_count == 0) {
		printk(KERN_INFO "Driver: read %d char\n", msg_len);
		msg_len = 0;
		return 0;	
	} else {
		printk(KERN_INFO "Fail to read %d char\n", error_count);
		error_count = 0;
		return -EFAULT;
	}
}
static ssize_t my_write(struct file *f, const char __user *buffer, size_t len,loff_t *off)
{
/*	sprintf(message, "%s", buffer);
	msg_len = strlen(message);
*/
	int error_count = 0;
	error_count = copy_from_user(message, buffer, len);
	if (error_count == 0) {
		msg_len = strlen(message);
		printk(KERN_INFO "Driver: write() %lu char\n", len);
		return len;
	} else {
		printk(KERN_INFO "Fail to read %d char\n", error_count);
		return -EFAULT;
	}
}
