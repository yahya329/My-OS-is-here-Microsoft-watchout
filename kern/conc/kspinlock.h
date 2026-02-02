// Mutual exclusion lock.
/*originally taken from xv6-x86 OS
 * USED ONLY FOR PROTECTION IN MULTI-CORE
 * Not designed for protection in a single core
 * */
#ifndef KERN_CONC_KSPINLOCK_H_
#define KERN_CONC_KSPINLOCK_H_

#include <inc/types.h>
#include <inc/stdio.h>

struct kspinlock {
  uint32 locked;       	// Is the lock held?

  // For debugging:
  char name[NAMELEN];	// Name of lock.
  struct cpu *cpu;   	// The cpu holding the lock.
  uint32 pcs[10];      	// The call stack (an array of program counters)
                     	// that locked the lock.
};
void init_kspinlock(struct kspinlock *lk, char *name);
void acquire_kspinlock(struct kspinlock *lk);
void release_kspinlock(struct kspinlock *lk);
int getcallerpcs(void *v, uint32 pcs[]) ;
void printcallstack(struct kspinlock *lk);
int holding_kspinlock(struct kspinlock *lock);
#endif /*KERN_CONC_KSPINLOCK_H_*/
