#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

asmlinkage int sys_printk(pid_t id, struct timespec *start, struct timespec *end)
{
	return printk("[Project1] %d %lu.%09lu %lu.%09lu\n", id, start->tv_sec, start->tv_nsec, end->tv_sec, end->tv_nsec);
}
