// Kernel-level Semaphore

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "ksemaphore.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_ksemaphore(struct ksemaphore *ksem, int value, char *name)
{
	init_channel(&(ksem->chan), "ksemaphore channel");
	init_kspinlock(&(ksem->lk), "lock of ksemaphore");
	strcpy(ksem->name, name);
	ksem->count = value;
}

void wait_ksemaphore(struct ksemaphore *ksem)
{
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #6 SEMAPHORE - wait_ksemaphore
	acquire_kspinlock(&ksem->lk);
	ksem->count--;
	if(ksem->count < 0)
	{
		sleep(&ksem->chan , &ksem->lk); // process is blocked and moved to the waiting queue
	}
	release_kspinlock(&ksem->lk);
	//Comment the following line
	//panic("wait_ksemaphore() is not implemented yet...!!");
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif

}

void signal_ksemaphore(struct ksemaphore *ksem)
{
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #7 SEMAPHORE - signal_ksemaphore
	acquire_kspinlock(&ksem->lk);
	ksem->count++;
	if(ksem->count <= 0)
	{
		wakeup_one(&ksem->chan); // process is moved from the blocked queue into the ready queue
	}
	release_kspinlock(&ksem->lk);
	//Comment the following line
	//panic("signal_ksemaphore() is not implemented yet...!!");
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}


