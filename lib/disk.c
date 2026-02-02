/*
 * Minimal PIO-based (non-interrupt-driven) IDE driver code.
 * For information about what all this IDE/ATA magic means,
 * see the materials available on the class references page.
 *
 * 2024: INTERRUPT-based is added to the IDE driver code (el7 :))
 */

#include <inc/disk.h>
#include <inc/x86.h>
#include <inc/trap.h>
#include <kern/trap/trap.h>
#include <kern/proc/user_environment.h>

#define IDE_BSY		0x80
#define IDE_DRDY	0x40
#define IDE_DF		0x20
#define IDE_ERR		0x01

static int diskno = 0;

void disk_interrupt_handler(struct Trapframe *tf)
{
	int r;
	//cprintf("\n>>>>>>>> DISK INTERRUPT <<<<<<<<<\n");
	if (((r = inb(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
	{
		//cprintf("NOT READY\n");
	}
	else
	{
#if DISK_IO_METHOD == INT_SLEEP
		wakeup_one(&DISKchannel);
#elif DISK_IO_METHOD == INT_SEMAPHORE
		signal_ksemaphore(&DISKsem);
#endif
	}

}

void ide_init()
{
	//irq_install_handler(15, &disk_interrupt_handler);
#if DISK_IO_METHOD == INT_SLEEP
	{
		irq_install_handler(14, &disk_interrupt_handler);
		init_channel(&DISKchannel, "DISK channel");
		init_kspinlock(&DISKlock, "DISK channel lock");
		init_sleeplock(&DISKmutex, "DISK mutex");
	}
#elif DISK_IO_METHOD == INT_SEMAPHORE
	{
		irq_install_handler(14, &disk_interrupt_handler);
		init_ksemaphore(&DISKsem, 0, "DISK semaphore");
		init_ksemaphore(&DISKmutex, 1, "DISK mutex");
	}
#endif
}


static int ide_wait_ready(bool check_error)
{
	int r;
	//cprintf("ide_wait_ready: begin\n");

#if DISK_IO_METHOD == PROGRAMMED_IO
	while (((r = inb(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
		/* do nothing */;
#else
	//If there's NO env (i.e. kernel in command prompt), then use PROGRAMMED-IO
	//since there's no env to be blocked!
	if (get_cpu_proc() == NULL)
	{
		while (((r = inb(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
			/* do nothing */;
	}
	else
	{
		if (((r = inb(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
		{
#if DISK_IO_METHOD == INT_SLEEP
			//should sleep (i.e. blocked) until a IRQ14 (Primary IDE) interrupt occur
			//cprintf("ide_wait_ready: before acquire_kspinlock\n");
			acquire_kspinlock(&DISKlock);
			{
				//cprintf("\n[%d] Will be BLOCKED\n", get_cpu_proc()->env_id);
				sleep(&DISKchannel, &DISKlock);
			}
			release_kspinlock(&DISKlock);
#elif DISK_IO_METHOD == INT_SEMAPHORE
			wait_ksemaphore(&DISKsem);
#endif
		}
	}
#endif
	if (check_error && (r & (IDE_DF|IDE_ERR)) != 0)
	{
		panic("ERROR @ ide_wait_ready() = %x(%d)\n",r,r);
		LOG_STATMENT(cprintf("ERROR @ ide_wait_ready() = %x(%d)\n",r,r););
		return -1;
	}
	//cprintf("ide_wait_ready: end\n");

	return 0;
}

int	ide_read(uint32 secno, void *dst, uint32 nsecs)
{
	int r;

	assert(nsecs <= 256);

	//TODODONE'24 el7: FUTURE NOTE: This BUSY-WAIT should be replaced by Interrupt to allow the OS to schedule another process till the device become ready [el7 :)]
	struct Env* e = get_cpu_proc();
	/*If there's env, Critical Section to ensure that the entire read/write will be completely finished*/
	if (e)
	{
		LOG_STATMENT(cprintf("ide_read: %d before CS\n", e->env_id););

#if DISK_IO_METHOD == INT_SLEEP
		acquire_sleeplock(&DISKmutex);
#elif DISK_IO_METHOD == INT_SEMAPHORE
		wait_ksemaphore(&DISKmutex);
#endif
	}
	{
		if (e) LOG_STATMENT(cprintf("ide_read: %d inside CS\n", e->env_id););
		ide_wait_ready(0);

		outb(0x1F2, nsecs);
		outb(0x1F3, secno & 0xFF);
		outb(0x1F4, (secno >> 8) & 0xFF);
		outb(0x1F5, (secno >> 16) & 0xFF);
		outb(0x1F6, 0xE0 | ((diskno&1)<<4) | ((secno>>24)&0x0F));
		outb(0x1F7, 0x20);	// CMD 0x20 means read sector

		for (; nsecs > 0; nsecs--, dst += SECTSIZE) {
			if ((r = ide_wait_ready(1)) < 0)
			{
				panic("FAILURE to read %d sectors to disk\n",nsecs);
				return r;
			}
			insl(0x1F0, dst, SECTSIZE/4);
		}
	}
	/*If there's env, Exit the Critical Section */
	if (e)
	{
		LOG_STATMENT(cprintf("ide_read: %d Left CS\n", e->env_id););

#if DISK_IO_METHOD == INT_SLEEP
		release_sleeplock(&DISKmutex);
#elif DISK_IO_METHOD == INT_SEMAPHORE
		signal_ksemaphore(&DISKmutex);
#endif
	}

	return 0;
}

int ide_write(uint32 secno, const void *src, uint32 nsecs)
{
	int r;

	//LOG_STATMENT(cprintf("1 ==> nsecs = %d\n",nsecs);)
	assert(nsecs <= 256);

	struct Env* e = get_cpu_proc();
	/*If there's env, Critical Section to ensure that the entire read/write will be completely finished*/
	if (e)
	{
		LOG_STATMENT(cprintf("ide_write: %d before CS\n", e->env_id););

#if DISK_IO_METHOD == INT_SLEEP
		//cprintf("ide_write: before acquire_sleeplock\n");
		acquire_sleeplock(&DISKmutex);
		//cprintf("ide_write: after acquire_sleeplock\n");
#elif DISK_IO_METHOD == INT_SEMAPHORE
		wait_ksemaphore(&DISKmutex);
#endif
	}
	{
		if (e) LOG_STATMENT(cprintf("ide_write: %d inside CS\n", e->env_id););

		ide_wait_ready(0);

		//LOG_STATMENT(cprintf("3 ==> nsecs = %d\n",nsecs);)
		outb(0x1F2, nsecs);
		outb(0x1F3, secno & 0xFF);
		outb(0x1F4, (secno >> 8) & 0xFF);
		outb(0x1F5, (secno >> 16) & 0xFF);
		outb(0x1F6, 0xE0 | ((diskno&1)<<4) | ((secno>>24)&0x0F));
		outb(0x1F7, 0x30);	// CMD 0x30 means write sector


		for (; nsecs > 0; nsecs--, src += SECTSIZE) {
			if ((r = ide_wait_ready(1)) < 0)
			{
				panic("FAILURE to write %d sectors to disk\n",nsecs);
				LOG_STATMENT(cprintf("FAILURE to write %d sectors to disk\n",nsecs););
				return r;
			}
			else
			{
				outsl(0x1F0, src, SECTSIZE/4);
				//LOG_STATMENT(cprintf("written %d sectors to disk successfully\n",nsecs););
			}
		}
	}
	/*If there's env, Exit the Critical Section */
	if (e)
	{
#if DISK_IO_METHOD == INT_SLEEP
		release_sleeplock(&DISKmutex);
#elif DISK_IO_METHOD == INT_SEMAPHORE
		signal_ksemaphore(&DISKmutex);
#endif

		LOG_STATMENT(cprintf("ide_write: %d Left CS\n", e->env_id););
	}
	//LOG_STATMENT(cprintf("5\n");)
	//cprintf("returning from ide_write \n");

	return 0;
}

