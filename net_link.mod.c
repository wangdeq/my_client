#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x964ee563, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x92272ab4, __VMLINUX_SYMBOL_STR(netlink_kernel_release) },
	{ 0x5419ed98, __VMLINUX_SYMBOL_STR(__netlink_kernel_create) },
	{ 0xd8486584, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0x84425d2a, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x73c15654, __VMLINUX_SYMBOL_STR(netlink_unicast) },
	{ 0x9d669763, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xe12c85e7, __VMLINUX_SYMBOL_STR(__nlmsg_put) },
	{ 0x6a643236, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3F1911F379FD13295240FAC");
