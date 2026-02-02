#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_kspinlock(&AllShares.shareslock, "shares lock");
	//init_sleeplock(&AllShares.sharessleeplock, "shares sleep lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//=========================
// [2] Find Share Object:
//=========================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* find_share(int32 ownerID, char* name)
{
#if USE_KHEAP
	struct Share * ret = NULL;
	bool wasHeld = holding_kspinlock(&(AllShares.shareslock));
	if (!wasHeld)
	{
		acquire_kspinlock(&(AllShares.shareslock));
	}
	{
		struct Share * shr ;
		LIST_FOREACH(shr, &(AllShares.shares_list))
		{
			if(shr->ownerID == ownerID && strcmp(name, shr->name)==0)
			{
				ret = shr;
				break;
			}
		}
	}
	if (!wasHeld)
	{
		release_kspinlock(&(AllShares.shareslock));
	}
	return ret;
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [3] Get Size of Share Object:
//==============================
int size_of_shared_object(int32 ownerID, char* shareName)
{
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = find_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}
//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=====================================
// [1] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* alloc_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{

#if USE_KHEAP
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #1 alloc_share
	//Your code is here

	int32 reqPages;
	reqPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;


	struct Share *sObj = kmalloc(sizeof(struct Share));
	if(!sObj){
		return NULL;
	}

	sObj->references = 1;
	sObj->ownerID = ownerID;
	strncpy(sObj->name, shareName,63);
	sObj->isWritable = isWritable;
	sObj->size = size;

	sObj->framesStorage = kmalloc(sizeof(struct FrameInfo*) * reqPages);
	if(!sObj->framesStorage){
		kfree(sObj);
		return NULL;
	}
	for(int i=0; i < reqPages; i++)
		sObj->framesStorage[i] = NULL;

	return sObj;
	//Comment the following line
	//panic("alloc_share() is not implemented yet...!!");
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif

}


//=========================
// [4] Create Share Object:
//=========================
int create_shared_object(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{

#if USE_KHEAP
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #3 create_shared_object
	//Your code is here

	struct Env* myenv = get_cpu_proc(); //The calling environment
	uint32 *ptr_page_table;
	int perm_flags = PERM_PRESENT | PERM_USER | PERM_WRITEABLE;
	virtual_address = ROUNDDOWN(virtual_address, PAGE_SIZE);

	if(find_share(ownerID, shareName) != NULL){
		return E_SHARED_MEM_EXISTS;
	}

	struct Share *sObj = alloc_share(ownerID, shareName, size, isWritable);
	if(sObj == NULL){
		return E_NO_SHARE;
	}


	uint32 reqPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;


	for(uint32 i = 0; i < reqPages; i++){

		uint32 frame_va = (uint32)virtual_address + i * PAGE_SIZE;
		allocate_user_mem(myenv, frame_va, PAGE_SIZE);


		int x = allocate_frame(&sObj->framesStorage[i]);
		if(x != 0)
			panic("ERROR: Kernel run out of memory... allocate_frame cannot find a free frame.\n");


		map_frame(myenv->env_page_directory, sObj->framesStorage[i], frame_va, perm_flags);

		sObj->framesStorage[i] = get_frame_info(myenv->env_page_directory, frame_va, &ptr_page_table);

		if(sObj->framesStorage[i] == NULL){

			for(uint32 n = 0; n <= i; n++){
				uint32 allocated_va = (uint32)virtual_address + n * PAGE_SIZE;
				free_user_mem(myenv, allocated_va, PAGE_SIZE);

			}

			kfree(sObj->framesStorage);
			kfree(sObj);
			return E_NO_SHARE;

		}

	}

	sObj->ID = ((uint32)virtual_address) & 0x7FFFFFFF;

	acquire_kspinlock(&AllShares.shareslock);
	LIST_INSERT_TAIL(&AllShares.shares_list, sObj);
	release_kspinlock(&AllShares.shareslock);

	return sObj->ID;

	//Comment the following line
	//panic("create_shared_object() is not implemented yet...!!");


	// This function should create the shared object at the given virtual address with the given size
	// and return the ShareObjectID
	// RETURN:
	//	a) ID of the shared object (its VA after masking out its msb) if success
	//	b) E_SHARED_MEM_EXISTS if the shared object already exists
	//	c) E_NO_SHARE if failed to create a shared object
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif

}


//======================
// [5] Get Share Object:
//======================
int get_shared_object(int32 ownerID, char* shareName, void* virtual_address)
{

#if USE_KHEAP

	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #5 get_shared_object
	//Your code is here

	struct Env* myenv = get_cpu_proc(); //The calling environment

	virtual_address = (void*)ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);

	acquire_kspinlock(&AllShares.shareslock);

	struct Share *sObj = find_share(ownerID, shareName);

	if(sObj == NULL){
		release_kspinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}

	//uint32 size = sObj->size;
	if(sObj->size == 0){
		release_kspinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}

	uint32 reqPages = ROUNDUP(sObj->size, PAGE_SIZE) / PAGE_SIZE;
	uint32 start_va = (uint32)virtual_address;
	uint32 end_va = start_va + reqPages * PAGE_SIZE;

	if(!(start_va >= USER_HEAP_START && end_va <= USER_HEAP_MAX)){
		release_kspinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}


    if (sObj->framesStorage == NULL) {
        release_kspinlock(&AllShares.shareslock);
        return E_SHARED_MEM_NOT_EXISTS;
    }

	int perm_flags = PERM_USER | (sObj->isWritable ? PERM_WRITEABLE : 0);

	for(uint32 i = 0; i < reqPages; i++){

	    if (sObj->framesStorage[i] == NULL) {
	        release_kspinlock(&AllShares.shareslock);
	        return E_SHARED_MEM_NOT_EXISTS;
	    }

		uint32 frame_va = (uint32)virtual_address + i * PAGE_SIZE;
		map_frame(myenv->env_page_directory, sObj->framesStorage[i], frame_va, perm_flags);
	}

	sObj->references++ ;

	int ret_id = sObj->ID;
	release_kspinlock(&AllShares.shareslock);

	return sObj->ID;

	//Comment the following line
	//panic("get_shared_object() is not implemented yet...!!");


	// 	This function should share the required object in the heap of the current environment
	//	starting from the given virtual_address with the specified permissions of the object: read_only/writable
	// 	and return the ShareObjectID
	// RETURN:
	//	a) ID of the shared object (its VA after masking out its msb) if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//
//=========================
// [1] Delete Share Object:
//=========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'25.BONUS#5] EXIT #2 - free_share
	//Your code is here
	//Comment the following line
	panic("free_share() is not implemented yet...!!");
}


//=========================
// [2] Free Share Object:
//=========================
int delete_shared_object(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'25.BONUS#5] EXIT #2 - delete_shared_object
	//Your code is here
	//Comment the following line
	panic("delete_shared_object() is not implemented yet...!!");

	struct Env* myenv = get_cpu_proc(); //The calling environment

	// This function should free (delete) the shared object from the User Heapof the current environment
	// If this is the last shared env, then the "frames_store" should be cleared and the shared object should be deleted
	// RETURN:
	//	a) 0 if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists

	// Steps:
	//	1) Get the shared object from the "shares" array (use get_share_object_ID())
	//	2) Unmap it from the current environment "myenv"
	//	3) If one or more table becomes empty, remove it
	//	4) Update references
	//	5) If this is the last share, delete the share object (use free_share())
	//	6) Flush the cache "tlbflush()"

}
