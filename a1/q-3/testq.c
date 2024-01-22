#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/init.h>

#define DEVICE_NAME "testq"
#define BUFFER_SIZE 1024
#define SUCCESS 0

MODULE_LICENSE("GPL");

static int major_number;
static char *char_buffer;
static int *msg_Ptr;
static int device_count = 0;
static struct mutex chardev_mutex;

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset);
static loff_t device_seek(struct file *file, loff_t offset, int whence);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .llseek = device_seek,
};

static int  __init chardev_init(void){
    char_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!char_buffer) {
        printk(KERN_ERR "Failed to allocate memory for buffer\n");
        return -ENOMEM;
    }

    mutex_init(&chardev_mutex);
    printk(KERN_INFO "inside %s function\n",__FUNCTION__);
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", major_number);
        return major_number;
    }
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major_number);

    return SUCCESS;
}

static void __exit chardev_exit(void)
{
    printk(KERN_INFO "Unregistering...\n");
    unregister_chrdev(major_number, DEVICE_NAME);
    kfree(char_buffer);
    printk(KERN_INFO "Unregistered the character device\n");
}

static int device_release(struct inode *inode, struct file *file)
    {
    printk(KERN_INFO "This device is now closed");
    mutex_lock(&chardev_mutex);
    device_count = device_count - 1; /* We're now ready for our next caller */
    /*
    * Decrement the usage count, or else once you opened the file,
    * you’ll never get get rid of the module.
    */
    mutex_unlock(&chardev_mutex);
    return 0;
    }

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "This device is now open");
    mutex_lock(&chardev_mutex);
    device_count = device_count + 1;
    mutex_unlock(&chardev_mutex);
    return SUCCESS;
}

static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset){
    int bytes_read = 0;
    mutex_lock(&chardev_mutex);
    if (*offset >= BUFFER_SIZE) {
        return 0; 
    }

    bytes_read = min(length, (size_t)(BUFFER_SIZE - *offset));

    if (copy_to_user(buffer, char_buffer + *offset, bytes_read) != 0) {
        return -EFAULT;
    }

    *offset += bytes_read;
    mutex_unlock(&chardev_mutex);
    
    return bytes_read;
}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset){
    int bytes_written = 0;

    // Copy data from user space to the buffer
    printk(KERN_INFO "Begin Writing...\n");

    mutex_lock(&chardev_mutex);

    bytes_written = min(length, (size_t)(BUFFER_SIZE - 1 - *offset));

    if (copy_from_user(char_buffer + *offset, buffer, bytes_written) != 0) {
        return -EFAULT; // Bad address
    }

    *offset += bytes_written;
    char_buffer[*offset + bytes_written] = '\0';

    mutex_unlock(&chardev_mutex);
    return bytes_written;
}

static loff_t device_seek(struct file *file, loff_t offset, int whence) {
    loff_t new_offset = 0;
    mutex_lock(&chardev_mutex);
    switch (whence) {
        case 0: // SEEK_SET
            new_offset = offset;
            break;
        case 1: // SEEK_CUR
            new_offset = file->f_pos + offset;
            break;
        case 2: // SEEK_END
            new_offset = BUFFER_SIZE + offset;
            break;
        default:
            return -EINVAL;
    }

    if (new_offset < 0 || new_offset > BUFFER_SIZE) {
        return -EINVAL;
    }

    file->f_pos = new_offset;
    mutex_unlock(&chardev_mutex);
    return new_offset;
}

module_init(chardev_init);
module_exit(chardev_exit);

