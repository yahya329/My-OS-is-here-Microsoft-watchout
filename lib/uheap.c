#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//==============================================
// [1] INITIALIZE USER HEAP:
//==============================================
//============DATA STRUCTURE============//
struct Region
{
    uint32 VA;
    uint32 size;
    bool is_F;//CHECK PAGE IS USED OR UNUSED
    LIST_ENTRY(Region) prev_next_info;
};

LIST_HEAD(Region_List, Region);
struct Region_List RegionList;

//=====EXACT FIT =============//

void* e_ff(struct Region* REG, uint32 All_SZ)
{
	REG->is_F = 0;
    sys_allocate_user_mem(REG->VA, All_SZ);//ALLOCATE FROM KERNEL
    return (void*)REG->VA;
}
//======WORST FIT ============//
void* w_ff(struct Region* REG, uint32 All_S)
{
    if (REG->size == All_S)
    {
    	REG->is_F = 0;
        sys_allocate_user_mem(REG->VA, All_S);
        return (void*)REG->VA;
    }
    else
    {
        uint32 remaining_size = ((REG->size) - All_S);
        // CREATE REGION
        struct Region* N_R = alloc_block(sizeof(struct Region));
        if (N_R == NULL)
            return NULL;
        N_R->VA = REG->VA + All_S;
        N_R->size = remaining_size;
        N_R->is_F = 1;
        LIST_INSERT_AFTER(&RegionList, REG, N_R);

        // UPDATE THE REGION - Corrected: list comes first, then listelm, then elm
        REG->size = All_S;
        REG->is_F = 0;
        sys_allocate_user_mem(REG->VA, All_S);
        return (void*)REG->VA;
    }
}

int __firstTimeFlag = 1;
void uheap_init()
{
	if(__firstTimeFlag)
	{
		initialize_dynamic_allocator(USER_HEAP_START, USER_HEAP_START + DYN_ALLOC_MAX_SIZE);
		uheapPlaceStrategy = sys_get_uheap_strategy();
		uheapPageAllocStart = dynAllocEnd + PAGE_SIZE;
		uheapPageAllocBreak = uheapPageAllocStart;

		__firstTimeFlag = 0;
		LIST_INIT(&RegionList);
	}
}

//==============================================
// [2] GET A PAGE FROM THE KERNEL FOR DA:
//==============================================
int get_page(void* va)
{
	int ret = __sys_allocate_page(ROUNDDOWN(va, PAGE_SIZE), PERM_USER|PERM_WRITEABLE|PERM_UHPAGE);
	if (ret < 0)
		panic("get_page() in user: failed to allocate page from the kernel");
	return 0;
}

//==============================================
// [3] RETURN A PAGE FROM THE DA TO KERNEL:
//==============================================
void return_page(void* va)
{
	int ret = __sys_unmap_frame(ROUNDDOWN((uint32)va, PAGE_SIZE));
	if (ret < 0)
		panic("return_page() in user: failed to return a page to the kernel");
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{

#if USE_KHEAP
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'25.IM#2] USER HEAP - #1 malloc
	//Your code is here
	 //=========BLOCK ALLOCATOR =========//
	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)return alloc_block(size);

	    uint32 exact_size = ROUNDUP(size, PAGE_SIZE); //(size+PAGE_SIZE-1)/PAGE_SIZE;
	    struct Region *exact = NULL, *worst = NULL;

	    for(struct Region *Reg = LIST_FIRST(&RegionList); Reg != NULL; Reg = LIST_NEXT(Reg))
	    {
	        if(Reg->is_F == 0) continue;

	        if(Reg->size == exact_size)//CHECK FOR EXACT FIT
	        {
	            exact = Reg;
	            break;
	        }

	        if(Reg->size > exact_size)//CHECK FOR WORST FIT
	        {
	            if(worst == NULL || Reg->size > worst->size)
	            {
	                worst = Reg;
	            }
	        }
	    }


	    if(exact != NULL)
	        return e_ff(exact, exact_size);

	    if(worst != NULL)
	        return w_ff(worst, exact_size);

	    // EXTEND BREAK
	    uint32 base = uheapPageAllocBreak;
	    if(base > USER_HEAP_MAX -exact_size)
	        return NULL;

	    struct Region* N_RE = alloc_block(sizeof(struct Region));
	    N_RE->VA = base;
	    N_RE->size = exact_size;
	    N_RE->is_F = 0;

	    LIST_INSERT_TAIL(&RegionList, N_RE);

	    sys_allocate_user_mem(base, exact_size);
	    uheapPageAllocBreak += exact_size;//MOVE BREAK UP

	    return (void*)base;
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif
}
//=================================
// [2] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{

#if USE_KHEAP
	//TODO: [PROJECT'25.IM#2] USER HEAP - #3 free
	//Your code is here
	 if (virtual_address == NULL)
	        return;


       uint32 va=ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
	    //=========BLOCK ALLOCATOR ===============//
	    if (va >= USER_HEAP_START && va < dynAllocEnd)
	    	return free_block(virtual_address);
	    //==========PAGE ALLOCATOR =============//
	    if (va < uheapPageAllocStart || va >= uheapPageAllocBreak)
	        panic(" virtual_address not fount");
	    //region
	    struct Region *reg = NULL;
	    LIST_FOREACH(reg, &RegionList)
	    {
	        if (reg->VA == va && reg->is_F == 0)
	            break;
	    }

	    if (reg == NULL || reg->is_F == 1)return;

	    //free page file
	    sys_free_user_mem(reg->VA, reg->size);
	       reg->is_F = 1;

	       //========MERGE REGION===========//

	       struct Region *NT = LIST_NEXT(reg);//MERGE WITH NEXTREGION IF FREE
	       if ((NT != NULL) && (NT->is_F == 1) && (reg->VA + reg->size == NT->VA))
	       {
	           reg->size += NT->size;
	           LIST_REMOVE(&RegionList, NT);
	           free_block(NT);
	       }

	       struct Region *P_V = LIST_PREV(reg);//MERGE WITH PREVIOUS REGION IF FREE
	       if ((P_V != NULL) &&( P_V->is_F == 1) && (P_V->VA + P_V->size == reg->VA))
	       {
	    	   P_V->size += reg->size;
	           LIST_REMOVE(&RegionList, reg);
	           free_block(reg);
	           reg = P_V;
	       }
          //========SHRINK HEAP IF TOP IS FREE==============//
	       if (reg->is_F == 1 && (reg->VA + reg->size) == uheapPageAllocBreak)
	       {
	           uheapPageAllocBreak = reg->VA;//MOVE BREAK DOWN
	           LIST_REMOVE(&RegionList, reg);
	           free_block(reg);
	       }
	//Comment the following line
	//panic("free() is not implemented yet...!!");
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif

}
//=================================
// [3] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{

#if USE_KHEAP
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	if (size == 0) return NULL ;
	//==============================================================

	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #2 smalloc
	//Your code is here
	uint32 bSize = ROUNDUP(size, PAGE_SIZE);
	uint32 va = 0;

	struct Region *exact = NULL;
	struct Region *worst = NULL;
	for(struct Region *myReg = LIST_FIRST(&RegionList); myReg != NULL; myReg = LIST_NEXT(myReg)){
		if(!myReg->is_F)
			continue;
		if(myReg->size == bSize){
			exact = myReg;
			break;
		}

		if(myReg->size > bSize){
			if(worst == NULL || myReg->size > worst->size)
				worst = myReg;
		}

	}

	if(exact){
		va = (uint32)e_ff(exact, bSize);

	}else if(worst){
		va = (uint32)w_ff(worst, bSize);

	}else{
		//extend break
		va = uheapPageAllocBreak;
		if(va + bSize > USER_HEAP_MAX)
			return NULL;

		struct Region *myNewReg = alloc_block(sizeof(struct Region));
		if(myNewReg == NULL)
			return NULL;
		myNewReg->VA = va;
		myNewReg->size = bSize;
		myNewReg->is_F = 0;
		LIST_INSERT_TAIL(&RegionList, myNewReg);

		sys_allocate_user_mem(va, bSize);
		uheapPageAllocBreak += bSize;

	}

	int check = sys_create_shared_object(sharedVarName, bSize, isWritable, (void*)va);

	if(check < 0)
		return NULL;

	return (void*)va;


	//Comment the following line
	//panic("smalloc() is not implemented yet...!!");
#else
panic("kmalloc: USE_KHEAP disabled!");
#endif
}

//========================================
// [4] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{

#if USE_KHEAP
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	//==============================================================

	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #4 sget
	//Your code is here
	uint32 size = sys_size_of_shared_object(ownerEnvID, sharedVarName);
	if(size == E_SHARED_MEM_NOT_EXISTS || size == 0)//obj isn't exist
		return NULL;

	uint32 bSize = ROUNDUP(size, PAGE_SIZE);
	uint32 va = 0;

	struct Region *	exact = NULL;
	struct Region *worst = NULL;
	for(struct Region *myReg = LIST_FIRST(&RegionList); myReg != NULL; myReg = LIST_NEXT(myReg)){
		if(!myReg->is_F)
			continue;
		if(myReg->size == bSize){
			exact = myReg;
			break;
		}

		if(myReg->size > bSize){
			if(worst == NULL || myReg->size > worst->size)
				worst = myReg;
		}

	}

	if(exact){
		va = (uint32)e_ff(exact, bSize);
	}else if(worst){
		va = (uint32)w_ff(worst, bSize);
	}else{
		//extend break
		va = uheapPageAllocBreak;

		if(va + bSize > USER_HEAP_MAX) //No Space Found
			return NULL;

		struct Region *myNewReg = alloc_block(sizeof(struct Region));
		if(myNewReg == NULL)
			return NULL;
		myNewReg->VA = va;
		myNewReg->size = bSize;
		myNewReg->is_F = 0;
		LIST_INSERT_TAIL(&RegionList, myNewReg);

		sys_allocate_user_mem(va, bSize);
		uheapPageAllocBreak += bSize;
	}

	if(va == 0) //LLta2keed
		return NULL;

	uint32 check = sys_get_shared_object(ownerEnvID, sharedVarName, (void*)va);
	if(check < 0){
		sys_free_user_mem(va, bSize);
		return NULL;
	}
	return (void*)va;

	//Comment the following line
	//panic("sget() is not implemented yet...!!");

#else
panic("kmalloc: USE_KHEAP disabled!");
#endif
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	//==============================================================
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_delete_shared_object(...); which switches to the kernel mode,
//	calls delete_shared_object(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the delete_shared_object() function is empty, make sure to implement it.
void sfree(void* virtual_address)
{//TODO: [PROJECT'25.BONUS#5] EXIT #2 - sfree
	//Your code is here
	//Comment the following line
	panic("sfree() is not implemented yet...!!");
	//	1) you should find the ID of the shared variable at the given address
	//2) you need to call sys_freeSharedObject()
}
//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

