#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/special_insns.h>
#include <asm/processor-flags.h>
#include <asm/unistd.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define MAX_SYSCALLS NR_syscalls

typedef long (*sys_call_ptr_t)(const struct pt_regs *);

static sys_call_ptr_t *real_sys_call_table;
static sys_call_ptr_t *static_sys_call_table;
static int original_length=0;
static sys_call_ptr_t *current_sys_call_table;
static sys_call_ptr_t orig_kill;

static unsigned long sys_call_table_addr;
module_param(sys_call_table_addr, ulong, 0);
MODULE_PARM_DESC(sys_call_table_addr, "Address of sys_call_table");

static const char *my_path = "/tmp/sysconf.txt";

static void write_syscall_table_to_file(void) {
    struct file *file;
    int i;

    file = filp_open("/tmp/sysconf.txt", O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(file)) {
        pr_info("Failed to open the file\n");
        return;
    }

    for (i = 0; i < NR_syscalls; i++) {
        sys_call_ptr_t syscall_func = static_sys_call_table[i];
        kernel_write(file, &syscall_func, sizeof(sys_call_ptr_t), &file->f_pos);
    }

    filp_close(file, NULL);
}

static void read_syscall_table_from_file(void) {
    struct file *file;
    int i;

    file = filp_open("/tmp/sysconf.txt", O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_info("Failed to open the file\n");
        return;
    }

    for (i = 0; i < NR_syscalls; i++) {
        sys_call_ptr_t syscall_func;
        kernel_read(file, &syscall_func, sizeof(sys_call_ptr_t), &file->f_pos);
        static_sys_call_table[i] = syscall_func;
    }

    filp_close(file, NULL);
}




static int file_exists(const char *path) {
    struct file *file;
    int err;

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_info("I'm Failed to open the file- Returning 0\n");
        return 0;  // The file does not exist or there was an error accessing it.
    }
    return 1;  // The file exists and can be opened.// The file does not exist or there was an error accessing it.
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
    int result = 0;
    result = file_exists(my_path);

    if (result == 0){
        pr_info("writing into file");
        memcpy(static_sys_call_table, real_sys_call_table, NR_syscalls * sizeof(sys_call_ptr_t));
    }
    else {
        pr_info("reading from file");
        read_syscall_table_from_file();
    }

    pr_info("init\n");

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


    return 0;
}

static void __exit modexit(void)
{

    pr_info("exit\n");

    if (file_exists(my_path) == 0){
        write_syscall_table_to_file();
    }

}

module_init(modinit);
module_exit(modexit);

MODULE_LICENSE("GPL");
