#ifndef FOS_SHARED_MEMORY_MANAGER_H
#define FOS_SHARED_MEMORY_MANAGER_H

#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

//#include <inc/types.h>
//#include <inc/queue.h>
//#include <inc/trap.h>
//#include <inc/memlayout.h>
#include <inc/environment_definitions.h>
#include "../conc/kspinlock.h"
#include <kern/conc/sleeplock.h>

struct Share
{
	//Unique ID for this Share object
	//Should be set to the VA of the created object
	//after masking the most significant bit (to make it +ve)
	int32 ID ;
	//ID of the owner environment
	int32 ownerID ;
	//share name
	char name[64];
	//share size
	int size;
	//references, number of envs looking at this shared mem object
	uint32 references;
	//sharing permissions (0: ReadOnly, 1:Writable)
	uint8 isWritable;
	//to store frames to be shared
	struct FrameInfo** framesStorage;
	// list link pointers
	LIST_ENTRY(Share) prev_next_info;

};

//List of all shared objects
LIST_HEAD(Share_List, Share);		// Declares 'struct Share_List'


#if USE_KHEAP == 0
	//max number of shared objects
	#define MAX_SHARES 100
	struct Share shares[MAX_SHARES] ;
#else
	struct
	{
		struct Share_List shares_list ;	//List of all share variables created by any process
		struct kspinlock shareslock;		//Use it to protect the shares_list in the kernel
		//struct sleeplock sharessleeplock;	//Use it to protect the shares_list in the kernel
	}AllShares;
	void sharing_init();
#endif

int size_of_shared_object(int32 ownerID, char* shareName);
int create_shared_object(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address);
int get_shared_object(int32 ownerID, char* shareName, void* virtual_address);
int delete_shared_object(int32 sharedObjectID, void *startVA);

#endif /* FOS_SHARED_MEMORY_MANAGER_H */
