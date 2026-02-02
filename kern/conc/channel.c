/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct kspinlock* lk)
{


#if USE_KHEAP
    struct Env *p  =  get_cpu_proc(); //current process
    acquire_kspinlock(&(ProcessQueues.qlock));
    enqueue(&chan->queue, p); //push it in channel
    p->env_status = ENV_BLOCKED; //process is blocked
    p->channel = chan;
    release_kspinlock(lk);
    sched();
    release_kspinlock(&(ProcessQueues.qlock));
    acquire_kspinlock(lk);
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}




//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel *chan)
{
#if USE_KHEAP
    // Get one blocked process
	acquire_kspinlock(&(ProcessQueues.qlock));
    struct Env *p = dequeue(&(chan->queue));
//    if (p == NULL)
//        return;
    p->env_status = ENV_READY;////process is ready
    p->channel = NULL;  // Clear channel
    sched_insert_ready(p); //scheduler ready queue
    release_kspinlock(&(ProcessQueues.qlock));
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_all(struct Channel *chan)
{
#if USE_KHEAP

    // Keep waking up everything in the queue
	acquire_kspinlock(&(ProcessQueues.qlock));

    while (queue_size(&(chan->queue)) > 0)
    {
    	struct Env *p = dequeue(&(chan->queue));
        p->env_status = ENV_READY; //process is ready
        p->channel = NULL; //clear channel
        sched_insert_ready(p);  //ready queue

    }
    release_kspinlock(&(ProcessQueues.qlock));
#else
    panic("kmalloc: USE_KHEAP disabled!");
#endif
}


