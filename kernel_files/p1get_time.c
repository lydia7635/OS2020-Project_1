#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
asmlinkage void sys_get_time(struct timespec *td)
{
	getnstimeofday(td);
	return;
}
