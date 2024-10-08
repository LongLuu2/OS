/*
 *  ioctl test module -- Rich West.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>


//#include <sys/io.h>
//#include <sys/ddi.h>

MODULE_LICENSE("GPL");

/* attribute structures */
struct ioctl_test_t {
  int field1;
  char field2;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define IOCTL_Keyboard _IOR(0, 7, char)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static struct proc_dir_entry *proc_keyboard_entry;

static irqreturn_t keyboard_interrupt(int irq, void *dev_id);

char c;
int char_flag = 0;


static int __init initialization_routine(void) {

  int result;
  printk("<1> Loading module\n");
  // disable_irq(1);
  // free_irq(1, NULL); //idk this workin ??
  // enable_irq(1);

  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
  /* Start create proc entry */
  proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  proc_keyboard_entry = create_proc_entry("keyboard_driver", 0444, NULL);
  if(!proc_keyboard_entry) {
    printk("<1> Error creating /proc entry for keyboard_driver.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;
  proc_keyboard_entry -> proc_fops = &pseudo_dev_proc_operations;

  result = request_irq(1, keyboard_interrupt, IRQF_SHARED, "keyboard_driver", (void *)(keyboard_interrupt));
  if (result) {
    printk(KERN_ERR "Error requesting IRQ 1: %d\n", result);
    return result;  
}

  return 0;
}

/* 'printk' version that prints to active tty. */
void my_printk(char *string)
{
  struct tty_struct *my_tty;

  my_tty = current->signal->tty;

  if (my_tty != NULL) {
    (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
    (*my_tty->driver->ops->write)(my_tty, "\n", 1);
  }
} 

static void __exit cleanup_routine(void) {

  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_test", NULL);
  remove_proc_entry("keyboard_driver", NULL);
  free_irq(1, (void *)(keyboard_interrupt));

  return;
}
///////////////////////////////////////////////
static inline unsigned char inb( unsigned short usPort ) {

    unsigned char uch;
   
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {

    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

char my_getchar ( void ) {

  char cc;

  static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

  cc = 'c';
  /* Poll keyboard status register at port 0x64 checking bit 0 to see if
   * output buffer is full. We continue to poll if the msb of port 0x60
   * (data port) is set, as this indicates out-of-band data or a release
   * keystroke
   */
  // while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );
  return c;
  // return scancode[ (int)cc ];

}

static irqreturn_t keyboard_interrupt(int irq, void *dev_id) {
    ///im going able to enter into this function thur caplock
  char key;


  key = my_getchar();

    c = key;
    char_flag= 1;
    my_printk("Keyboard interrupt triggered\n");

    return IRQ_HANDLED;
}

/////////////////////////////////////////////////////////////

/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
  struct ioctl_test_t ioc;
  char key;
  
  switch (cmd){

  case IOCTL_TEST:
    copy_from_user(&ioc, (struct ioctl_test_t *)arg, 
		   sizeof(struct ioctl_test_t));
    printk("<1> ioctl: call to IOCTL_TEST (%d,%c)!\n", 
	   ioc.field1, ioc.field2);

    my_printk ("Got msg in kernel\n");
    break;
  
  case IOCTL_Keyboard:
    while(!char_flag){
      ssleep(1);
    }
    key = 'b';
    char_flag = 0;
    printk("<1> ioctl: call to IOCTL_Keyboard, got char: %c\n", key);
    my_printk("Got msg in kernel(Keyboard_driver)");
    copy_to_user((char __user *)arg, &key, sizeof(char));  
    
    break;
  
  default:
    return -EINVAL;
    break;
  }
  
  return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine); 