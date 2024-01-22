#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/special_insns.h>
#include <asm/processor-flags.h>
#include <asm/unistd.h>
#include <linux/vmalloc.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

typedef long (*sys_call_ptr_t)(const struct pt_regs *);

static sys_call_ptr_t *real_sys_call_table;
static sys_call_ptr_t *static_sys_call_table;
static sys_call_ptr_t *current_sys_call_table;
static sys_call_ptr_t orig_kill;
static int original_length=0;

struct syscall_info {
    const char *name;
    asmlinkage long (*function)(struct pt_regs *);
};

static unsigned long sys_call_table_addr;
module_param(sys_call_table_addr, ulong, 0);
MODULE_PARM_DESC(sys_call_table_addr, "Address of sys_call_table");

// Since Linux v5.3, [native_]write_cr0 won't change "sensitive" CR0 bits.
static void write_cr0_unsafe(unsigned long val)
{
    asm volatile("mov %0,%%cr0": "+r" (val) : : "memory");
}

static long my_kill(const struct pt_regs *regs)
{
    int sig = regs->si;
    if (sig == 64){
    int j = 0;
    for(j = 0 ; j < NR_syscalls; j ++){
        if (real_sys_call_table[j] != static_sys_call_table[j]){
            pr_info("!!!Sys call %d intercepted!!!", j);
        }
    }
    if (NR_syscalls > original_length) {
        int k = 0;
        for (k = 0 ; k < (NR_syscalls - original_length) ; k ++){
            pr_info("***Sys call %d is new***", k);
        }
    }
}
     else
    {
        return orig_kill(regs);
    }
    return 0;   
}

static int __init modinit(void)
{
    unsigned long old_cr0;
    int i = 0;

    real_sys_call_table = (typeof(real_sys_call_table))sys_call_table_addr;
    static_sys_call_table = (sys_call_ptr_t *)vmalloc(NR_syscalls * sizeof(sys_call_ptr_t));
    original_length = NR_syscalls;

    if (!static_sys_call_table) {
        pr_info("Failed to allocate memory for static_sys_call_table\n");
        return -ENOMEM;
    }

    memcpy(static_sys_call_table, real_sys_call_table, NR_syscalls * sizeof(sys_call_ptr_t));

    pr_info("init\n");

    old_cr0 = read_cr0();
    write_cr0_unsafe(old_cr0 & ~(X86_CR0_WP));

    orig_kill = real_sys_call_table[__NR_kill];
    real_sys_call_table[__NR_kill] = my_kill;

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

    real_sys_call_table[__NR_kill] = orig_kill;

    write_cr0_unsafe(old_cr0);

    pr_info("goodbye\n");
}

module_init(modinit);
module_exit(modexit);

MODULE_LICENSE("GPL");
