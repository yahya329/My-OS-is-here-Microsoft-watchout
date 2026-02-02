// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	char prefix[30] = "lock of sleeplock - ";
	char guardName[30+NAMELEN];
	strcconcat(prefix, name, guardName);
	init_kspinlock(&(lk->lk), guardName);
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}

void acquire_sleeplock(struct sleeplock *lk)
{
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #4 SLEEP LOCK - acquire_sleeplock

	acquire_kspinlock(&lk->lk);//guard in lec
	while(lk->locked == 1) //lock is busy
	{

		sleep(&lk->chan,&lk->lk); //process sleep call

	}
	lk->locked = 1; // lock is busy
	release_kspinlock(&lk->lk);

	//Comment the following line
	//panic("acquire_sleeplock() is not implemented yet...!!");
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}

void release_sleeplock(struct sleeplock *lk)
{
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #5 SLEEP LOCK - release_sleeplock
	acquire_kspinlock(&lk->lk);//guard in lec
	if(!queue_size(&lk->chan.queue) == 0)
	{
		wakeup_all(&lk->chan); //wake up call fun
	}
	lk->locked = 0; //lock is free
	release_kspinlock(&lk->lk);
	//Comment the following line
	//panic("release_sleeplock() is not implemented yet...!!");
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}

int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_kspinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_kspinlock(&(lk->lk));
	return r;
}



