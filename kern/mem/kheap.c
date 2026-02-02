#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include <kern/conc/sleeplock.h>
#include <kern/proc/user_environment.h>
#include <kern/mem/memory_manager.h>
#include "../conc/kspinlock.h"
#include <inc/queue.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//==============================================
// [1] INITIALIZE KERNEL HEAP:
//==============================================
//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #0 kheap_init [GIVEN]
//Remember to initialize locks (if any)
void kheap_init() {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		initialize_dynamic_allocator(KERNEL_HEAP_START,
				KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE);
		set_kheap_strategy(KHP_PLACE_CUSTOMFIT);
		kheapPageAllocStart = dynAllocEnd + PAGE_SIZE;
		kheapPageAllocBreak = kheapPageAllocStart;
		LIST_INIT(&KRegionList);
	}
	//==================================================================================
	//==================================================================================
}

//==============================================
// [1] HELPER FUNCTIONS :
//==============================================

//=====EXACT FIT =============//
void* exactFit_kernel(struct Region* R, uint32 S) {
	R->isFree = 0;

	for (uint32 v = R->VA; v < R->VA + S; v += PAGE_SIZE)
		alloc_page(ptr_page_directory, v, PERM_WRITEABLE, 1);

	return (void*) R->VA;
}

//======WORST FIT ============//
void* worstFit_kernel(struct Region* R, uint32 S) {
	if (R->size == S) {
		return exactFit_kernel(R, S);
	} else {
		uint32 remaining = R->size - S;
		// CREATE REGION
		struct Region* NewR = (struct Region*) alloc_block(
				sizeof(struct Region));
		if (!NewR) {
			return NULL;
		}
		NewR->VA = R->VA + S;
		NewR->size = remaining;
		NewR->isFree = 1;
		// UPDATE THE REGION - Corrected: list comes first, then listelm, then elm
		LIST_INSERT_AFTER(&KRegionList, R, NewR);
		R->size = S;
		R->isFree = 0;

		for (uint32 v = R->VA; v < R->VA + S; v += PAGE_SIZE)
			alloc_page(ptr_page_directory, v, PERM_WRITEABLE, 1);

		return (void*) R->VA;
	}
}

void add_reg(uint32 size, uint32 va) {
	struct Region *reg = (struct Region*) alloc_block(sizeof(*reg));

	reg->VA = va;
	reg->size = size;
	reg->isFree = 0;
	LIST_INSERT_TAIL(&KRegionList, reg);

}

//==============================================
// [2] GET A PAGE FROM THE KERNEL FOR DA:
//==============================================
int get_page(void* va) {
	int ret = alloc_page(ptr_page_directory, ROUNDDOWN((uint32 )va, PAGE_SIZE),
			PERM_WRITEABLE, 1);
	if (ret < 0)
		panic("get_page() in kern: failed to allocate page from the kernel");
	return 0;
}

//==============================================
// [3] RETURN A PAGE FROM THE DA TO KERNEL:
//==============================================
void return_page(void* va) {
	unmap_frame(ptr_page_directory, ROUNDDOWN((uint32 )va, PAGE_SIZE));
}

//==================================================================================//
//============================ REQUIRED FUNCTINS ==================================//
//==================================================================================//
//===================================
// [1] ALLOCATE SPACE IN KERNEL HEAP:
//===================================
void* kmalloc(unsigned int size) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #1 kmalloc
	//Your code is here
# if USE_KHEAP
	if (size == 0)
		return NULL;

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		return alloc_block(size);

	uint32 exact_size = ROUNDUP(size, PAGE_SIZE); //(size+PAGE_SIZE-1)/PAGE_SIZE;

	struct Region *exact = NULL;
	struct Region *worst = NULL;

	for (struct Region* R = LIST_FIRST(&KRegionList); R != NULL;
			R = LIST_NEXT(R)) {
		if (R->isFree == 0)
			continue;

		if (R->size == exact_size) //CHECK FOR EXACT FIT
				{
			exact = R;
			break;
		}
		if (R->size > exact_size) //CHECK FOR WORST FIT
				{
			if (worst == NULL || R->size > worst->size)
				worst = R;
		}
	}

	if (exact != NULL) {
		return exactFit_kernel(exact, exact_size);
	}

	if (worst != NULL) {
		return worstFit_kernel(worst, exact_size);
	}

	//extend break

	uint32 base = kheapPageAllocBreak;

	if (base > KERNEL_HEAP_MAX - exact_size)
		return NULL;

	// allocate pages from break -> break + exact_size

	for (uint32 va = base; va < base + exact_size; va += PAGE_SIZE) {
		int ret = alloc_page(ptr_page_directory, va, PERM_WRITEABLE, 1);
		if (ret < 0) {
			// rollback on failure
			for (uint32 r = base; r < va; r += PAGE_SIZE)
				return_page((void*) r);
			return NULL;
		}
	}

	kheapPageAllocBreak += exact_size;

	add_reg(exact_size, base);

	return (void*) base;

#else
	panic("kmalloc: USE_KHEAP disabled!");
#endif

	//Comment the following line
	//panic_into_prompt("kmalloc() is not implemented yet...!!");

	//TODO: [PROJECT'25.BONUS#3] FAST PAGE ALLOCATOR
}

//=================================
// [2] FREE SPACE FROM KERNEL HEAP:
//=================================
void kfree(void* va) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #2 kfree
	//Your code is here
#if USE_KHEAP
	if (va == NULL)
		return;

	uint32 address = ROUNDDOWN((uint32 )va, PAGE_SIZE);

	//BLK allocator
	if (address >= KERNEL_HEAP_START && address < dynAllocEnd) {
		free_block(va);
		return;
	}

	if (address >= kheapPageAllocStart && address < kheapPageAllocBreak) {

		struct Region *reg = NULL;

		LIST_FOREACH(reg, &KRegionList)
		{
			if (reg->VA == address && reg->isFree == 0)
				break;
		}

		if (reg == NULL || reg->isFree == 1)
			return;

		//free each page
		for (uint32 i = 0; i < reg->size; i += PAGE_SIZE)
			return_page((void*) (reg->VA + i));

		reg->isFree = 1;

		// Merge with next region if free
		struct Region *next = LIST_NEXT(reg);
		if (next != NULL && next->isFree == 1
				&& reg->VA + reg->size == next->VA) {
			reg->size += next->size;
			LIST_REMOVE(&KRegionList, next);
			free_block(next);
		}

		// Merge with previous region if free(by iterating)
		struct Region *PV = NULL;
		struct Region *iter;
		LIST_FOREACH(iter, &KRegionList)
		{
			if (LIST_NEXT(iter) == reg) {
				PV = iter;
				break;
			}
		}

		if (PV != NULL && PV->isFree == 1 && PV->VA + PV->size == reg->VA) {
			PV->size += reg->size;
			LIST_REMOVE(&KRegionList, reg);
			free_block(reg);
			reg = PV;
		}

		// Shrink heap if top is free
		if (reg->isFree == 1 && reg->VA + reg->size == kheapPageAllocBreak) {
			kheapPageAllocBreak = reg->VA;
			LIST_REMOVE(&KRegionList, reg);
			free_block(reg);
		}
	}
#else
	panic("kfree: USE_KHEAP disabled!");
#endif
	//Comment the following line
	//panic("kfree() is not implemented yet...!!");
}
//=================================
// [3] FIND VA OF GIVEN PA:
//=================================
unsigned int kheap_virtual_address(unsigned int physical_address) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #3 kheap_virtual_address
	//Your code is here
#if USE_KHEAP
	struct FrameInfo *F_i = to_frame_info(physical_address);

	if (F_i != NULL && F_i->va != 0) {
		uint32 virtual_address = F_i->va + PGOFF(physical_address);

		if (virtual_address >= KERNEL_HEAP_START
				&& virtual_address < KERNEL_HEAP_MAX) {
			return virtual_address;
		}
	}

	return 0;
#else
	panic("kheap_virtual_address: USE_KHEAP disabled!");
#endif
	//Comment the following line
	//panic("kheap_virtual_address() is not implemented yet...!!");

	/*EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED */
}

//=================================
// [4] FIND PA OF GIVEN VA:
//=================================
unsigned int kheap_physical_address(unsigned int virtual_address) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #4 kheap_physical_address
	//Your code is here
#if USE_KHEAP
	if (virtual_address >= KERNEL_HEAP_START
			&& virtual_address < KERNEL_HEAP_MAX) //check if va in kheap
	{
		uint32 *ptr_T = NULL;
		struct FrameInfo *ptr_F = NULL;
		ptr_F = get_frame_info(ptr_page_directory, virtual_address, &ptr_T);
		if (ptr_F == NULL)
			return 0;
		return (to_physical_address(ptr_F) + (PGOFF(virtual_address))); //return pa->(frame+offset)
	}
	return 0;
#else
	panic("kheap_physical_address: USE_KHEAP disabled!");
#endif
	//Comment the following line
	//panic("kheap_physical_address() is not implemented yet...!!");

	/*EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED */
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

extern __inline__ uint32 get_block_size(void *va);

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT'25.BONUS#2] KERNEL REALLOC - krealloc
	//Your code is here
#if USE_KHEAP
	if(virtual_address==NULL)
		return kmalloc(new_size);

	if(new_size==0){
		kfree(virtual_address);
		return NULL;
	}

	uint32 addr =(uint32)virtual_address;

    if (addr >= KERNEL_HEAP_START && addr < dynAllocEnd) {

        uint32 old = get_block_size(virtual_address);

        // shrink in place
        if (new_size <= old)
            return virtual_address;

        void *newptr = kmalloc(new_size);
        if (newptr == NULL)
            return NULL;

        memcpy(newptr, virtual_address, old);
        free_block(virtual_address);

        return newptr;
    }


    uint32 A = ROUNDDOWN(addr, PAGE_SIZE);
    struct Region *R = NULL;

    LIST_FOREACH(R, &KRegionList) {
        if (R->VA == A && R->isFree == 0)
            break;
    }

    if (R == NULL)
        return NULL;

    uint32 old_size = R->size;
    uint32 new_aligned = ROUNDUP(new_size, PAGE_SIZE);

    if (new_aligned <= old_size) {

        uint32 reduce = old_size - new_aligned;

        if (reduce > 0) {
            // free tail pages
            for (uint32 va = R->VA + new_aligned; va < R->VA + old_size; va += PAGE_SIZE)
                return_page((void*)va);

            struct Region *NR = alloc_block(sizeof(*NR));
            NR->VA = R->VA + new_aligned;
            NR->size = reduce;
            NR->isFree = 1;

            LIST_INSERT_AFTER(&KRegionList, R, NR);

            struct Region *NX = LIST_NEXT(NR);
            if (NX && NX->isFree == 1 && NR->VA + NR->size == NX->VA) {
                NR->size += NX->size;
                LIST_REMOVE(&KRegionList, NX);
                free_block(NX);
            }
        }

        R->size = new_aligned;
        return virtual_address;
    }

    struct Region *next = LIST_NEXT(R);
    uint32 needed_more = new_aligned - old_size;

    if (next && next->isFree == 1 && next->size >= needed_more) {

        // allocate pages for the new extension
        for (uint32 va = R->VA + old_size; va < R->VA + new_aligned; va += PAGE_SIZE) {
            alloc_page(ptr_page_directory, va, PERM_WRITEABLE, 1);
        }

        // shrink next region
        next->VA += needed_more;
        next->size -= needed_more;

        if (next->size == 0) {
            LIST_REMOVE(&KRegionList, next);
            free_block(next);
        }

        R->size = new_aligned;
        return virtual_address;
    }

    void *newptr = kmalloc(new_size);
    if (newptr == NULL)
        return NULL;

    memcpy(newptr, virtual_address, old_size);
    kfree(virtual_address);

    return newptr;
#else
	panic("kmalloc: USE_KHEAP disabled!");
#endif
	//Comment the following line
	//panic("krealloc() is not implemented yet...!!");
}
