/*

	chardev.c: Creates a read-only char device that says how many times
		   you've read from the dev file

*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

//prototypes that would normally go in a .h file

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev" //dev name as it appears in /proc/devices
#define BUFF_LEN 80 //max length of the message from the device

//global variables are declared as static, so are global within the file

static int Major; //major number assingned to device driver
static int Device_Open = 0; //check if device open to prevent multiple access
static char msg[BUFF_LEN]; //the msg the device will give when asked
static char *msg_ptr;

static struct file_operations fops = {

	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release

};

//function is called when module is loaded
int init_module(void){

	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if(Major < 0){

		printk(KERN_ALERT"Registering char device failed with %d/n", Major);
		return Major;	

	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;

}

void cleanup_module(void){


	//unregistering device
	unregister_chrdev(Major, DEVICE_NAME);
	/*if(ret < 0)
		printk(KERN_ALERT"Error in unregister_chrdev: %d\n", ret);*/

}

//Methods of use

//called when process tries to open the device file
//like "cat /dev/mrcharfile"

static int device_open(struct inode *inode, struct file *filp){

	static int counter = 0;

	if(Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;

}

//called when a process closes the device file

static int device_release(struct inode *inode, struct file *filp){

	Device_Open--;

	//decrement usage count. or else once you opened the file, you'll
	//never get rid of the module
	module_put(THIS_MODULE);
	
	return 0;

}

//called when a process, which already opened the dev file, attempts
//to read from it

static ssize_t device_read(struct  file *filp, //see include/linux/fs.h
			   char *buffer, //buffer to fill data
			   size_t length, //length of buffer
			   loff_t *offset){

	//number of bytes written to buffer
	int bytes_read = 0;

	//if we're at the end of the message, return 0
	if(*msg_ptr == 0)
		return 0;

	//actually put the data into the buffer
	while(length && *msg_ptr){
		//buffer is in the user data segment, no the kernel
		//segment so "*" assignment won't work. We have to use
		// put_user which copies data from the kernel data seg
		// to the user data segment
		put_user(*(msg_ptr++), buffer++);
		length--;
		bytes_read++;
		

	}
	//most read functions return the number of bytes put into the buffer
	return bytes_read;

}

//called when a process writes to dev file
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off){

	printk(KERN_ALERT"Sorry, this operation isn't supported.\n");
	return -EINVAL;

}
