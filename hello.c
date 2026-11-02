// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Romanenko");
MODULE_DESCRIPTION("Hello World kernel module with timestamps");

static unsigned int hello_count = 1;

module_param(hello_count, uint, 0444);
MODULE_PARM_DESC(hello_count, "Number of Hello World messages");

struct hello_event {
	struct list_head list;
	ktime_t timestamp;
};

static LIST_HEAD(hello_events);

static int __init hello_init(void)
{
	struct hello_event *event, *tmp;
	unsigned int i;

	if (hello_count > 10) {
		pr_err("hello: count %u exceeds 10\n", hello_count);
		return -EINVAL;
	}

	if (hello_count == 0 || hello_count >= 5)
		pr_warn("hello: unusual count %u\n", hello_count);

	for (i = 0; i < hello_count; i++) {
		event = kmalloc(sizeof(*event), GFP_KERNEL);
		if (!event)
			goto cleanup;

		event->timestamp = ktime_get();
		list_add_tail(&event->list, &hello_events);

		pr_info("Hello, world!\n");
	}

	return 0;

cleanup:
	list_for_each_entry_safe(event, tmp, &hello_events, list) {
		list_del(&event->list);
		kfree(event);
	}

	return -ENOMEM;
}

static void __exit hello_exit(void)
{
	struct hello_event *event, *tmp;

	list_for_each_entry_safe(event, tmp, &hello_events, list) {
		pr_info("hello: timestamp = %lld ns\n",
			(long long)ktime_to_ns(event->timestamp));

		list_del(&event->list);
		kfree(event);
	}

	pr_info("Goodbye, world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
