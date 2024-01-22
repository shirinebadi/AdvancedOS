#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0xbec1f61d, "pv_ops" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x754d539c, "strlen" },
	{ 0x69acdf38, "memcpy" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xa19b956, "__stack_chk_fail" },
	{ 0x2ef9f811, "param_ops_ulong" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x453e7dc, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "743D58729A0A23E86702BBC");
