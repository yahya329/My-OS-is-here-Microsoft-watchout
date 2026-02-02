
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//==================================
//==================================
// [1] GET PAGE VA:
//==================================
__inline__ uint32 to_page_va(struct PageInfoElement *ptrPageInfo)//return the start va of the page
{
	if (ptrPageInfo < &pageBlockInfoArr[0] || ptrPageInfo >= &pageBlockInfoArr[DYN_ALLOC_MAX_SIZE/PAGE_SIZE])
			panic("to_page_va called with invalid pageInfoPtr");
	//Get start VA of the page from the corresponding Page Info pointer
	int idxInPageInfoArr = (ptrPageInfo - pageBlockInfoArr);//return page index
	return dynAllocStart + (idxInPageInfoArr << PGSHIFT);
}

//==================================
// [2] GET PAGE INFO OF PAGE VA:
//==================================
__inline__ struct PageInfoElement * to_page_info(uint32 va)
{
	int idxInPageInfoArr = (va - dynAllocStart) >> PGSHIFT;
	if (idxInPageInfoArr < 0 || idxInPageInfoArr >= DYN_ALLOC_MAX_SIZE/PAGE_SIZE)
		panic("to_page_info called with invalid pa");
	return &pageBlockInfoArr[idxInPageInfoArr];
}

static inline unsigned int nearest_pow2_ceil(unsigned int x) {
	if (x <= 1) return 1;
	int power = 2;
	x--;
	while (x >>= 1) {
		power <<= 1;
	}
	return power;
}

static inline unsigned int log2_ceil(unsigned int x) {
    unsigned int r = 0;
    while ((1U << r) < x) r++;
    return r;
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
bool is_initialized = 0;
void initialize_dynamic_allocator(uint32 daStart, uint32 daEnd)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert(daEnd <= daStart + DYN_ALLOC_MAX_SIZE);
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #1 initialize_dynamic_allocator
	//Your code is here

	//The Limits start and end of the heap (32 MB)
	dynAllocStart=daStart;
	dynAllocEnd=daEnd;

	int32 numOfpages = (daEnd - daStart) / PAGE_SIZE;


	for(int i=0;i<numOfpages;i++)
	{
	pageBlockInfoArr[i].block_size=0;
	pageBlockInfoArr[i].num_of_free_blocks=0;
	}

	//INIT the free page list (header) -> null
	LIST_INIT(&freePagesList);

	for(int i=0;i<numOfpages;i++)
	{
		LIST_INSERT_TAIL(&freePagesList,&pageBlockInfoArr[i]);
	}
	//number of free blocks lists 11-3+1=9
	int numOflists=LOG2_MAX_SIZE - LOG2_MIN_SIZE + 1;

	for (int i =0;i<numOflists;i++)
	{
		LIST_INIT(&freeBlockLists[i]);//init each list header ->null
	}


	//Comment the following line
	//panic("initialize_dynamic_allocator() Not implemented yet");

}

//===========================
// [2] GET BLOCK SIZE:
//===========================
uint32 get_block_size(void *va)
{
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #2 get_block_size
	//Your code is here

	    if ((uint32)va < dynAllocStart || (uint32)va >= dynAllocEnd)
	        return 0;

	    struct PageInfoElement* page = to_page_info((uint32)va);
	    if (!page) return 0;

	    return page->block_size;

	//Comment the following line
	//panic("get_block_size() Not implemented yet");
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================
void *alloc_block(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert(size <= DYN_ALLOC_MAX_BLOCK_SIZE);//check if size <=2kb
	}
	//==================================================================================
	//==================================================================================
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #3 alloc_block
	//Your code is here

	    if (size == 0) return NULL;
	    if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
	        size = DYN_ALLOC_MIN_BLOCK_SIZE;

	    uint32 BK_SZ = nearest_pow2_ceil(size);
	    uint32 ndx = log2_ceil(BK_SZ) - LOG2_MIN_SIZE;

	    // CASE 1: Free block exists
	    if (!LIST_EMPTY(&freeBlockLists[ndx])) {
	        struct BlockElement* block = LIST_FIRST(&freeBlockLists[ndx]);
	        LIST_REMOVE(&freeBlockLists[ndx], block);
	        struct PageInfoElement* page = to_page_info((uint32)block);
	        page->num_of_free_blocks--;
	        return (void*)block;
	    }

	    // CASE 2: Allocate new page
	    if (!LIST_EMPTY(&freePagesList)) {
	        struct PageInfoElement* page = LIST_FIRST(&freePagesList);
	        LIST_REMOVE(&freePagesList, page);

	        void* pg_va = (void*)to_page_va(page);
	        get_page(pg_va);

	        uint32 numBlocks = PAGE_SIZE / BK_SZ;

	        for (int i = 0; i < numBlocks; i++) {
	            struct BlockElement* block = (struct BlockElement*)((uint32)pg_va + i * BK_SZ);
	            LIST_INSERT_TAIL(&freeBlockLists[ndx], block);
	        }

	        page->block_size = BK_SZ;
	        page->num_of_free_blocks = numBlocks - 1;

	        struct BlockElement* block = LIST_FIRST(&freeBlockLists[ndx]);
	        LIST_REMOVE(&freeBlockLists[ndx], block);
	        return (void*)block;
	    }

	    // CASE 3: Find in larger lists
	    for (int largerIndex = ndx + 1; largerIndex <= (LOG2_MAX_SIZE - LOG2_MIN_SIZE); largerIndex++) {
	        if (!LIST_EMPTY(&freeBlockLists[largerIndex])) {
	            struct BlockElement* block = LIST_FIRST(&freeBlockLists[largerIndex]);
	            LIST_REMOVE(&freeBlockLists[largerIndex], block);
	            struct PageInfoElement* page = to_page_info((uint32)block);
	            page->num_of_free_blocks--;
	            return (void*)block;
	        }
	    }

	    // CASE 4: Nothing free
	    return NULL;


	//Comment the following line
	//panic("alloc_block() Not implemented yet");

	//TODO: [PROJECT'25.BONUS#1] DYNAMIC ALLOCATOR - block if no free block
}

//===========================
// [4] FREE BLOCK:
//===========================
void free_block(void *va)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert((uint32)va >= dynAllocStart && (uint32)va < dynAllocEnd);
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #4 free_block
	//Your code is here
	        //Find corresponding size from pageBlockkInfoArr
	  struct PageInfoElement* page = to_page_info((uint32)va);
		    if (!page) return;

		    uint32 bk_sz=page->block_size;
		    if(!bk_sz) return;
		    //Return block to the corresponding list
		    uint32 inx=log2_ceil(bk_sz)-LOG2_MIN_SIZE;
		    struct BlockElement* bk=(  struct BlockElement*)va;
		    LIST_INSERT_HEAD(&freeBlockLists[inx],bk);
		    //Increment free blocks in pageBlockkInfoArr
		    page->num_of_free_blocks++;
		    //If entire page becomes free
		    if(page->num_of_free_blocks==PAGE_SIZE/bk_sz)
		    {
		    	uint32 ps_va =to_page_va(page);
		    //Remove all its blocks from corresponding list in
		    	struct BlockElement* b_k=LIST_FIRST(&freeBlockLists[inx]);
		    	while(b_k!=NULL){
		    	 struct BlockElement* ex=LIST_NEXT(b_k);
		    	 if((uint32)b_k >=ps_va && (uint32)b_k < ps_va+PAGE_SIZE){
		    	 LIST_REMOVE(&freeBlockLists[inx],b_k);
		    	}
		    	b_k=ex;

		    	}
		    //return it to the free frame list
		    	return_page((void*)ps_va);
		    //add it to the freePagesList
		    	LIST_INSERT_TAIL(&freePagesList,page);
		    	page->num_of_free_blocks=0;
		        page->block_size=0;

		    }


	//Comment the following line
	//panic("free_block() Not implemented yet");
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] REALLOCATE BLOCK:
//===========================
void *realloc_block(void* va, uint32 new_size)
{
	//TODO: [PROJECT'25.BONUS#2] KERNEL REALLOC - realloc_block
	//Your code is here
	if (va==NULL)
		return alloc_block(new_size);

	if (new_size==0){
		free_block(va);
		return NULL;
	}

	uint32 BLK_size=get_block_size(va);

	if (BLK_size ==0)
		return NULL;

	uint32 required_size =new_size;

	if(required_size<DYN_ALLOC_MIN_BLOCK_SIZE)
	{
		required_size = DYN_ALLOC_MIN_BLOCK_SIZE;
	}

	required_size = nearest_pow2_ceil(required_size);

	void *new_va = alloc_block(new_size);

	if(new_va)
		return NULL;

	uint32 copy_size = (BLK_size < new_size) ? BLK_size : new_size;

	memcpy(new_va, va, copy_size);

	free_block(va);

	return new_va;

	//Comment the following line
	//panic("realloc_block() Not implemented yet");
}
