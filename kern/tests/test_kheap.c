#include "test_kheap.h"

#include <inc/memlayout.h>
#include <inc/queue.h>
#include <inc/dynamic_allocator.h>
#include <kern/cpu/sched.h>
#include <kern/disk/pagefile_manager.h>
#include "../mem/kheap.h"
#include "../mem/memory_manager.h"


/**********************************************************************************************/
/************************ GLOBAL DATA & FUNCTIONS FOR ALL TESTS *******************************/
/**********************************************************************************************/

#define Mega  (1024*1024)
#define kilo (1024)

#define INITIAL_PAGE_ALLOCATIONS (0)
#define ACTUAL_PAGE_ALLOC_START ((KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE) + INITIAL_PAGE_ALLOCATIONS)
#define INITIAL_BLOCK_ALLOCATIONS ((2*sizeof(int) + MAX(num_of_ready_queues * sizeof(uint8), DYN_ALLOC_MIN_BLOCK_SIZE)) + (2*sizeof(int) + MAX(num_of_ready_queues * sizeof(struct Env_Queue), DYN_ALLOC_MIN_BLOCK_SIZE)))
#define  BLOCK_ALLOC_LIMIT (KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE)

extern uint32 sys_calculate_free_frames() ;
extern void sys_bypassPageFault(uint8);
extern uint32 sys_rcr2();
extern int execute_command(char *command_string);
extern char end_of_kernel[];
extern int CB(uint32 *ptr_dir, uint32 va, int bn);


/*KMALLOC*/
int test_kmalloc_FF_page();
int test_kmalloc_NF_page();
int test_kmalloc_BF_page();
int test_kmalloc_WF_page();
int test_kmalloc_CF_page();

int test_kmalloc_FF_block();
int test_kmalloc_NF_block();
int test_kmalloc_BF_block();
int test_kmalloc_WF_block();
int test_kmalloc_CF_block();

int test_kmalloc_FF_both();
int test_kmalloc_NF_both();
int test_kmalloc_BF_both();
int test_kmalloc_WF_both();
int test_kmalloc_CF_both();

/*KFREE*/
int test_kfree_FF_page();
int test_kfree_NF_page();
int test_kfree_BF_page();
int test_kfree_WF_page();
int test_kfree_CF_page();

int test_kfree_FF_block();
int test_kfree_NF_block();
int test_kfree_BF_block();
int test_kfree_WF_block();
int test_kfree_CF_block();

int test_kfree_FF_both();
int test_kfree_NF_both();
int test_kfree_BF_both();
int test_kfree_WF_both();
int test_kfree_CF_both();

/*KREALLOC*/
int test_krealloc_FF_page();
int test_krealloc_NF_page();
int test_krealloc_BF_page();
int test_krealloc_WF_page();
int test_krealloc_CF_page();

int test_krealloc_FF_block();
int test_krealloc_NF_block();
int test_krealloc_BF_block();
int test_krealloc_WF_block();
int test_krealloc_CF_block();

int test_krealloc_FF_both();
int test_krealloc_NF_both();
int test_krealloc_BF_both();
int test_krealloc_WF_both();
int test_krealloc_CF_both();

/*FAST PAGE ALLOCATOR*/
int test_fast_FF();
int test_fast_NF();
int test_fast_BF();
int test_fast_WF();
int test_fast_CF();

int test_kmalloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_CF_both();
	}
	return 0;
}

int test_kfree(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_CF_both();
	}
	return 0;
}
int test_krealloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_CF_both();
	}
	return 0;
}

void* ptr_fast_allocations[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE] = {0};
int test_fast_page_alloc()
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		test_fast_FF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		test_fast_NF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		test_fast_BF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		test_fast_WF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		test_fast_CF();
	}
	return 0;
}


/**********************************************************************************************/
/*************************** INITIAL ALLOCATION FOR ALL TESTS *********************************/
/**********************************************************************************************/
#define MAX_NUM_OF_ALLOCS 20
void* ptr_allocations[MAX_NUM_OF_ALLOCS] = {0};
int lastIndices[MAX_NUM_OF_ALLOCS] = {0};
uint32 requestedSizes[MAX_NUM_OF_ALLOCS] = {0};
uint32 totalRequestedSize ;
bool allocSpaceInPageAlloc(int allocIndex, uint32 size, bool fill)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int freeDiskFrames = (int)pf_calculate_free_frames() ;
	requestedSizes[allocIndex] = size ;
	uint32 expectedNumOfFrames = ROUNDUP(requestedSizes[allocIndex], PAGE_SIZE) / PAGE_SIZE ;
	{
		ptr_allocations[allocIndex] = kmalloc(requestedSizes[allocIndex]);
	}
	if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
	if ((freeFrames - sys_calculate_free_frames()) < expectedNumOfFrames) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	lastIndices[allocIndex] = (size)/sizeof(char) - 1;
	if (fill)
	{
		char* ptr = (char*)ptr_allocations[allocIndex];
		for (int i = 0; i < lastIndices[allocIndex]; ++i)
		{
			ptr[i] = allocIndex ;
		}
	}
	return correct;

}

bool freeSpaceInPageAlloc(int allocIndex)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int freeDiskFrames = (int)pf_calculate_free_frames() ;
	uint32 expectedNumOfFrames = ROUNDUP(requestedSizes[allocIndex], PAGE_SIZE) / PAGE_SIZE ;
	{
		kfree(ptr_allocations[allocIndex]);
	}
	if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf("%d.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
	if ((sys_calculate_free_frames() - freeFrames) < expectedNumOfFrames) { correct = 0; cprintf("$d.2 Wrong kfree: pages in memory are not freed correctly\n", allocIndex); }
	return correct;
}
int initial_page_allocations()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	uint32 expectedVA = ACTUAL_PAGE_ALLOC_START;

	//malloc some spaces
	int i, freeFrames, freeDiskFrames, allocIndex ;
	uint32 size = 0;
	char* ptr;
	int sums[20] = {0};
	totalRequestedSize = 0;

	int eval = 0;
	bool correct ;

	correct = 1;
	//Create some areas in PAGE allocators
	cprintf_colored(TEXT_cyan,"\n	1.1 Create some areas in PAGE allocators\n");
	{
		//4 MB
		allocIndex = 0;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//3 MB
		allocIndex = 1;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 3*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 2;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//4 MB
		allocIndex = 3;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 4;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 3*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 5;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 2*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 6;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 1*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 7;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 8;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

	}
	//Insufficient space
	cprintf_colored(TEXT_cyan,"\n	1.2 Insufficient Space\n");
	{
		allocIndex = 9;
		expectedVA = 0;
		freeFrames = (int)sys_calculate_free_frames() ;
		freeDiskFrames = (int)pf_calculate_free_frames() ;
		uint32 restOfKHeap = (KERNEL_HEAP_MAX - ACTUAL_PAGE_ALLOC_START) - (4*Mega+3*Mega+2*Mega+4*Mega+3*Mega+2*Mega+2*Mega) ;
		ptr_allocations[allocIndex] = kmalloc(restOfKHeap+1);
		if (ptr_allocations[allocIndex] != NULL) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Allocating insufficient space: should return NULL\n", allocIndex); }
		if (((int)pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
		if ((freeFrames - (int)sys_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	}
	if (correct)	eval+=10 ;

	return eval;
}

short* startBlockVAs[DYN_ALLOC_MAX_SIZE / DYN_ALLOC_MIN_BLOCK_SIZE] = {0} ;
#define numOfLevels (LOG2_MAX_SIZE - LOG2_MIN_SIZE + 1)
int numOfAllocBlocksPerSize[numOfLevels] = {0};
int numOfAllocPages = 0;

int initial_block_allocations()
{
	memset(numOfAllocBlocksPerSize, 0, numOfLevels * sizeof(int)) ;
	numOfAllocPages = 0;

	int eval = 0;
	bool is_correct = 1;


	int freeFramesBefore = sys_calculate_free_frames();
	void *va ;
	//====================================================================//
	/*1: Check initial allocations (two blocks should be allocated at start-up 16B & 8B)*/
	cprintf_colored(TEXT_cyan, "\n	1.1: Check initial BLOCK allocations (two blocks should be allocated at start-up 16B & 8B)\n\n") ;
	uint32 numOfRemain8 = PAGE_SIZE / 8 - 1;
	uint32 numOfRemain16 = PAGE_SIZE / 16 - 1;
	if (LIST_SIZE(&freeBlockLists[0]) != numOfRemain8 || LIST_SIZE(&freeBlockLists[1]) != numOfRemain16)
	{
		is_correct = 0;
		cprintf_colored(TEXT_TESTERR_CLR, "initial allocation #1: unexpected list size! check boot-time allocation\n");
	}
	numOfAllocBlocksPerSize[0]++ ;
	numOfAllocBlocksPerSize[1]++ ;
	//====================================================================//
	/*2: Allocate set of blocks for each possible block size */
	cprintf_colored(TEXT_cyan, "	1.2: Allocate set of blocks for each possible block size \n\n") ;
	int curSize = 1<<LOG2_MIN_SIZE ;
	int curIndex = 0;
	int numOfBlksAtCurSize = 0;
	int maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
	uint32 currentVA = KERNEL_HEAP_START;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int freeDiskFrames = (int)pf_calculate_free_frames() ;

	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		//allocate a new block with the curSize
		{
			va = alloc_block(s);
			startBlockVAs[s] = va;
		}

		//fill the entire block
		for (int i = 0; i < curSize/2; ++i)
		{
			startBlockVAs[s][i] = s;
		}

		numOfAllocBlocksPerSize[curIndex]++ ;

		uint32 expectedVA = currentVA ;
		//special cases for the 8B & 16B blocks since they are allocated during the boot-time
		if (curSize == 8) expectedVA = KERNEL_HEAP_START + PAGE_SIZE;
		if (curSize == 16) expectedVA = KERNEL_HEAP_START ;
		if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "initial allocation #2.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", s, expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
		}
		if (s == curSize)
		{
			if (is_correct)	eval += 5;

			//Reinitialize
			{
				curSize <<= 1;
				curIndex++;
				currentVA += PAGE_SIZE;
				numOfAllocPages++;
				maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
				is_correct = 1;
			}
		}
		else if (numOfAllocBlocksPerSize[curIndex] % maxNumOfBlksAtCurPage == 0)
		{
			currentVA += PAGE_SIZE;
			numOfAllocPages++;
		}
	}

	//====================================================================//
	/*3: Check content of each block */
	cprintf_colored(TEXT_cyan, "	1.3: Check content of each block \n\n") ;
	curSize = 1<<LOG2_MIN_SIZE ;
	is_correct = 1;
	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		//check the content of the current block
		int sum = 0;
		for (int i = 0; i < curSize/2; ++i)
		{
			sum += startBlockVAs[s][i] ;
		}
		if (is_correct && sum != s * curSize/2)
		{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d wrong content\n", s); }

		if (s == curSize)
		{
			if (is_correct)	eval += 5;

			//Reinitialize
			{
				curSize <<= 1;
				is_correct = 1;
			}
		}
	}

	//====================================================================//
	/*4: Check number of allocated pages */
	cprintf_colored(TEXT_cyan, "	1.4: Check number of allocated pages \n\n") ;
	is_correct = 1;
	if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
	//exclude the 1st two pages that are allocated to 16B & 8B during boot-time
	if ((freeFrames - sys_calculate_free_frames()) != numOfAllocPages - 2) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", numOfAllocPages,freeFrames - sys_calculate_free_frames()); }
	if (is_correct)	eval += 10;

	return eval;
}

/**********************************************************************************************/
/********************************** KMALLOC TESTING AREA **************************************/
/**********************************************************************************************/
int test_kmalloc_FF_page()
{
	panic("not implemented function");
}
int test_kmalloc_NF_page()
{
	panic("not implemented function");
}
int test_kmalloc_BF_page()
{
	panic("not implemented function");
}
int test_kmalloc_WF_page()
{
	panic("not implemented function");
}

int test_kmalloc_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces
	int eval = 0;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces [70%]\n");
	{
		eval = initial_page_allocations();
		eval = eval * 70 / 100; //rescale
	}

	//2. Check BREAK
	int correct = 1;
	cprintf_colored(TEXT_cyan,"\n2. Check Page Allocator BREAK [10%]\n");
	{
		uint32 allocSizes = 0;
		for (int i = 0; i < 9; ++i)
		{
			allocSizes += ROUNDUP(requestedSizes[i], PAGE_SIZE);
		}
		uint32 expectedVA = ACTUAL_PAGE_ALLOC_START + allocSizes;
		if(kheapPageAllocBreak != expectedVA) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedVA, kheapPageAllocBreak);}
	}
	if (correct) eval += 10;
	correct = 1;

	//3. Check Permissions
	cprintf_colored(TEXT_cyan,"\n3. Check permissions of allocated spaces in PAGE ALLOCATOR [10%]\n");
	{
		uint32 lastAllocAddress = (uint32)ptr_allocations[8] + 2*Mega ;
		uint32 va;
		for (va = ACTUAL_PAGE_ALLOC_START; va < lastAllocAddress; va+=PAGE_SIZE)
		{
			unsigned int * table;
			get_page_table(ptr_page_directory, va, &table);
			uint32 perm = table[PTX(va)] & 0xFFF;
			if ((perm & PERM_USER) == PERM_USER)
			{
				if (correct)
				{
					correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong permissions: pages should be mapped with Supervisor permission only\n");
				}
			}
		}
	}
	if (correct) eval += 10;
	correct = 1;

	//4. Check Content
	uint32 sums[MAX_NUM_OF_ALLOCS] = {0};
	cprintf_colored(TEXT_cyan,"\n4. Check Content [10%]\n");
	{
		for (int i = 0; i < 9; ++i)
		{
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 10;
	correct = 1;

	cprintf_colored(TEXT_light_green,"\nTest kmalloc Page Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kmalloc_FF_block()
{
	panic("not implemented function");
}
int test_kmalloc_NF_block()
{
	panic("not implemented function");
}
int test_kmalloc_BF_block()
{
	panic("not implemented function");
}
int test_kmalloc_WF_block()
{
	panic("not implemented function");
}
int test_kmalloc_CF_block()
{
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces
	int eval = 0;
	cprintf_colored(TEXT_cyan,"1. Alloc some spaces [50%]\n");
	{
		eval = initial_block_allocations();
		eval = eval * 50 / 100; //rescale
	}
	//2. Fill-up the remaining pages using the 2KB block
	void *va;
	int is_correct;
	cprintf_colored(TEXT_cyan,"2. Fill-up the remaining pages using the 2KB block [15%]\n");
	{
		is_correct = 1;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		int requiredNumOf2KBBlks = (DYN_ALLOC_MAX_SIZE/PAGE_SIZE - numOfAllocPages) * 2;
		int curIndex = numOfLevels - 1;
		uint32 currentVA = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE ;
		for (int i = 1; i <= requiredNumOf2KBBlks; ++i)
		{
			//allocate a new block with the curSize
			{
				va = alloc_block(DYN_ALLOC_MAX_BLOCK_SIZE);
			}

			numOfAllocBlocksPerSize[curIndex]++ ;

			uint32 expectedVA = currentVA ;
			if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #5.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", i, expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
			}
			if (i%2 == 0)
			{
				numOfAllocPages++;
				currentVA += PAGE_SIZE;
			}
		}
		int expectedNumOfAllocPages = requiredNumOf2KBBlks / 2;
		if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		if ((freeFrames - sys_calculate_free_frames()) != expectedNumOfAllocPages) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", expectedNumOfAllocPages, freeFrames - sys_calculate_free_frames()); }

		if (is_correct) eval += 15;
		is_correct = 1;
	}

	//3. Allocate blocks of same size that consume remaining free blocks at all levels
	cprintf_colored(TEXT_cyan,"\n3. Allocate blocks of same size that consume remaining free blocks at all levels [25%]\n") ;
	{
		is_correct = 1;
		//calculate expected number of free blocks at all levels
		int curSize = DYN_ALLOC_MIN_BLOCK_SIZE;
		int numOfRemFreeBlks[numOfLevels] = {0};
		uint32 expectedPageIndex[numOfLevels] = {0};
		uint32 prevAllocPages = 0;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;

		for (int i = 0; i < numOfLevels; ++i)
		{
			int numOfAllocBlks = numOfAllocBlocksPerSize[i] ;
			int expectedNumOfBlksPerPage = PAGE_SIZE / curSize;
			if (numOfAllocBlks % expectedNumOfBlksPerPage != 0)
			{
				numOfRemFreeBlks[i] = expectedNumOfBlksPerPage - (numOfAllocBlks % expectedNumOfBlksPerPage) ;
				//Special cases for 16B & 8B blocks due to their allocation at the boot-time
				if (i == 0) //8B Block is in the 2nd page
				{
					expectedPageIndex[i] = 1;
				}
				else if (i == 1) //16B Block is in the 1st page
				{
					expectedPageIndex[i] = 0 ;
				}
				else
				{
					expectedPageIndex[i] = (prevAllocPages + numOfAllocBlks / expectedNumOfBlksPerPage) ;
				}
				prevAllocPages += numOfAllocBlks / expectedNumOfBlksPerPage + 1;
			}
			else
			{
				numOfRemFreeBlks[i] = 0;
				expectedPageIndex[i] = 0;
				prevAllocPages += numOfAllocBlks / expectedNumOfBlksPerPage;
			}
			curSize *= 2 ;
		}

		//Allocate a number of blocks of same size to consume all the remaining free blocks
		int blkSize = 1<<LOG2_MIN_SIZE ;
		for (int i = 0; i < numOfLevels; ++i)
		{
			uint32 expectedVA = KERNEL_HEAP_START + expectedPageIndex[i] * PAGE_SIZE;
			//cprintf( "Level#%d: num of remaining free blocks = %d\n", i, numOfRemFreeBlks[i]);
			for (int j = 0; j < numOfRemFreeBlks[i]; ++j)
			{
				va = alloc_block(blkSize);
				int *tmpVal = va ;
				*tmpVal = 353 ;
				if (ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #6.1: WRONG! VA is not correct (i = %d, j = %d)\n", i, j);
					break;
				}
				if (*tmpVal != 353)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #6.2: wrong stored value in the allocated block\n");
				}
			}

			if (numOfRemFreeBlks[i] > 0)
			{
				if (LIST_SIZE(&freeBlockLists[i]) != 0)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #6.3: WRONG! there's still free blocks at level %d while not expected to\n", i);
				}
				if (pageBlockInfoArr[expectedPageIndex[i]].num_of_free_blocks != 0)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #6.4: WRONG! there's still free blocks at page %d while not expected to\n", expectedPageIndex[i]);
				}
			}
		}
		int expectedNumOfAllocPages = 0;
		if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Block Alloc #6.5: Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		if ((freeFrames - sys_calculate_free_frames()) != expectedNumOfAllocPages) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Block Alloc #6.6: Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", expectedNumOfAllocPages, freeFrames - sys_calculate_free_frames()); }

		if (is_correct) eval += 25;
		is_correct = 1;
	}

	//4. Check Permissions
	cprintf_colored(TEXT_cyan,"\n4. Check permissions of allocated spaces in BLOCK ALLOCATOR [10%]\n");
	{
		uint32 lastAllocAddress = (uint32)ptr_allocations[8] + 2*Mega ;
		uint32 va;
		for (va = KERNEL_HEAP_START; va < KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE; va+=PAGE_SIZE)
		{
			unsigned int * table;
			get_page_table(ptr_page_directory, va, &table);
			uint32 perm = table[PTX(va)] & 0xFFF;
			if ((perm & PERM_USER) == PERM_USER)
			{
				if (is_correct)
				{
					is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Block Alloc #7: Wrong permissions: pages should be mapped with Supervisor permission only\n");
				}
			}
		}
	}
	if (is_correct) eval += 10;

	cprintf_colored(TEXT_light_green,"\nTest kmalloc Block Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kmalloc_FF_both()
{
	panic("not implemented function");
}
int test_kmalloc_NF_both()
{
	panic("not implemented function");
}
int test_kmalloc_BF_both()
{
	panic("not implemented function");
}
int test_kmalloc_WF_both()
{
	panic("not implemented function");
}
int test_kmalloc_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/*********************************** KFREE TESTING AREA ***************************************/
/**********************************************************************************************/
int test_kfree_FF_page()
{
	panic("not implemented function");
}
int test_kfree_NF_page()
{
	panic("not implemented function");
}
int test_kfree_BF_page()
{
	panic("not implemented function");
}
int test_kfree_WF_page()
{
	panic("not implemented function");
}
int test_kfree_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces in PAGE allocator
	int correct = 1;
	int eval;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in PAGE allocator\n");
	{
		eval = initial_page_allocations();
		if (eval != 100)
		{
			cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kfree\n");
			return 0;
		}
	}
	eval = 0;
	//2. Free some allocations to create initial holes
	cprintf_colored(TEXT_cyan,"\n2. Free some allocations to create initial holes [5%]\n");
	correct = 1;
	{
		//3 MB Hole
		correct = freeSpaceInPageAlloc(1);

		//2nd 4 MB Hole
		correct = freeSpaceInPageAlloc(3);

		//2nd 1 MB Hole
		correct = freeSpaceInPageAlloc(5);

		//2nd 2 MB Hole
		correct = freeSpaceInPageAlloc(7);
	}
	if (correct) eval += 5;
	correct = 1;

	//3. Check content of un-freed spaces
	uint32 sums[MAX_NUM_OF_ALLOCS] = {0};
	cprintf_colored(TEXT_cyan,"\n3. Check content of un-freed spaces [5%]\n");
	{
		for (int i = 0; i < 9; ++i)
		{
			//skip the freed spaces
			if (i == 1 || i == 3 || i == 5 || i == 7)
				continue;
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 5;
	correct = 1;

	//4. Check BREAK
	correct = 1;
	uint32 expectedBreak = 0;
	cprintf_colored(TEXT_cyan,"\n4. Check BREAK [5%]\n");
	{
		uint32 allocSizes = 0;
		for (int i = 0; i < 9; ++i)
		{
			allocSizes += ROUNDUP(requestedSizes[i], PAGE_SIZE);
		}
		expectedBreak = ACTUAL_PAGE_ALLOC_START + allocSizes;
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}
	}
	if (correct) eval += 5;
	correct = 1;

	//5. Allocate after kfree [Test CUSTOM FIT]
	uint32 allocIndex,expectedVA, size = 0;
	cprintf_colored(TEXT_cyan,"\n5. Allocate after kfree [Test CUSTOM FIT] [30%]\n");
	{
		//1 MB [EXACT FIT in 1MB Hole (alloc#5)]
		allocIndex = 10;
		size = 1*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] ; //Address of 1MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//1MB + 4KB [WORST FIT in 4MB Hole (alloc#3)]
		allocIndex = 11;
		size = 1*Mega + 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] ; //Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3MB - 4KB [EXACT FIT in remaining area of 4MB Hole (alloc#3)]
		allocIndex = 12;
		size = 3*Mega - 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] + 1*Mega + 4*kilo; //1MB.4KB after the Start Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//1.5 MB [WORST FIT in 3MB Hole (alloc#1)]
		allocIndex = 13;
		size = 1*Mega + Mega/2;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[1] ; //Address of 3MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//2.5 MB [EXTEND THE BREAK]
		allocIndex = 14;
		size = 2*Mega + Mega/2;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = expectedBreak ;
		expectedBreak += ROUNDUP(size, PAGE_SIZE);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}

		//Insufficient space
		allocIndex = 15;
		expectedVA = 0;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		uint32 restOfKHeap = (KERNEL_HEAP_MAX - ACTUAL_PAGE_ALLOC_START) - expectedBreak ;
		ptr_allocations[allocIndex] = kmalloc(restOfKHeap+1);
		if (ptr_allocations[allocIndex] != NULL) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Allocating insufficient space: should return NULL\n", allocIndex); }
		if (((int)pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
		if ((freeFrames - (int)sys_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	}
	if (correct) eval+=30;
	correct = 1;

	//6. Check content of newly allocated  spaces
	cprintf_colored(TEXT_cyan,"\n6. Check content of newly allocated spaces [5%]\n");
	{
		for (int i = 10; i < 15; ++i)
		{
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 5;
	correct = 1;

	//7. Free some allocations to create MERGED holes
	correct = 1;
	cprintf_colored(TEXT_cyan,"\n7. Free some allocations to create MERGED holes [10%]\n");
	{
		//Free new 3MB allocation inside the 4MB Hole
		correct = freeSpaceInPageAlloc(12);

		//Free new 1MB allocation at the beginning of the 4MB Hole (should be MERGED with next 3MB) => 4MB HOLE
		correct = freeSpaceInPageAlloc(11);

		//Free new 1MB allocation at the beginning of the 3MB Hole (should be MERGED with next 1.5MB) => 3MB HOLE
		correct = freeSpaceInPageAlloc(13);

		//Free new 1MB allocation at the 1MB Hole (NO MERGED)
		correct = freeSpaceInPageAlloc(10);

		//Free original 3rd 1MB allocation (should be MERGED with next 2MB hole and the prev 1MB hole) => 4MB HOLE
		correct = freeSpaceInPageAlloc(6);

		//Free original last 2MB allocation (should be MERGED with the prev 4MB created hole) => 6MB HOLE
		correct = freeSpaceInPageAlloc(8);
	}
	if (correct) eval += 10;
	correct = 1;

	//8. Allocate after kfree [Test CUSTOM FIT in MERGED FREE SPACES]
	cprintf_colored(TEXT_cyan,"\n8. Allocate after kfree [Test CUSTOM FIT in MERGED FREE SPACES] [20%]\n");
	{
		//3 MB [EXACT FIT in 3MB Hole]
		allocIndex = 16;
		size = 3*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[1] ; //Address of 3MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3 MB [WORST FIT in 6MB Hole]
		allocIndex = 17;
		size = 3*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] ; //Address of 6MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3MB - 4KB [WORST FIT in 4MB Hole]
		allocIndex = 18;
		size = 3*Mega - 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] ; //Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3 MB [EXACT FIT in remaining of 6MB Hole]
		allocIndex = 19;
		size = 3*Mega;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] + 3*Mega ; //3MB after the start address of 6MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
	}
	if (correct) eval += 20;
	correct = 1;

	//9. Free last allocations to move-down the BREAK
	correct = 1;
	cprintf_colored(TEXT_cyan,"\n9. Free last allocations to move-down the BREAK [10%]\n");
	{
		//Free last allocated 3MB
		correct = freeSpaceInPageAlloc(19);

		//Free 2.5MB allocation (should be merged with prev 3MB and the break should be moved-down)
		correct = freeSpaceInPageAlloc(14);

		expectedBreak = expectedBreak - (5*Mega + Mega/2) ;
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}
	}
	if (correct) eval += 10;
	correct = 1;

	//Check memory access of FREED area in PAGE allocator
	char* ptr;
	cprintf_colored(TEXT_cyan,"\n10. Check memory access of FREED area in PAGE allocator [5%]\n");
	{
		//Bypass the PAGE FAULT on <MOVB immediate, reg> instruction by setting its length
		//and continue executing the remaining code
		sys_bypassPageFault(3);

		ptr = (char *) ptr_allocations[18] + requestedSizes[18]; //begin of free 1MB after the 3rd 3MB allocation
		ptr[0] = 10;
		//cprintf("\n\ncr2 = %x, faulted addr = %x", sys_rcr2(), (uint32)&(ptr[0]));
		if (sys_rcr2() != (uint32)&(ptr[0]))
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"10 kfree: successful access to freed space!! it should not be succeeded\n"); }
		ptr[1*Mega-1] = 10;
		if (sys_rcr2() != (uint32)&(ptr[1*Mega-1]))
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"10 kfree: successful access to freed space!! it should not be succeeded\n"); }

		//set it to 0 again to cancel the bypassing option
		sys_bypassPageFault(0);
	}
	if (correct) eval += 5;
	correct = 1;

	correct = 1 ;
	//11. Check page tables
	cprintf_colored(TEXT_cyan,"\n11. check page tables [5%]\n");
	{
		long long va;
		for (va = KERNEL_HEAP_START; va < (long long)KERNEL_HEAP_MAX; va+=PTSIZE)
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, (uint32)va, &ptr_table);
			if (ptr_table == NULL)
			{
				if (correct)
				{ correct = 0; cprintf("Wrong kfree: one of the kernel tables is wrongly removed! Tables should not be removed here in kfree\n"); }
			}
		}
	}
	if (correct)	eval+=5 ;

	cprintf_colored(TEXT_light_green,"\nTest kfree & CUSTOM FIT Page Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kfree_FF_block()
{
	panic("not implemented function");
}
int test_kfree_NF_block()
{
	panic("not implemented function");
}
int test_kfree_BF_block()
{
	panic("not implemented function");
}
int test_kfree_WF_block()
{
	panic("not implemented function");
}
int test_kfree_CF_block()
{
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	int origFreeFrames = (int)sys_calculate_free_frames();
	int eval ;
	//1. Alloc some blocks at each possible block size
	cprintf_colored(TEXT_cyan,"\n1. Alloc some blocks at each possible block size\n");
	{
		eval = initial_block_allocations();
		if (eval != 100)
		{
			cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kfree\n");
			return 0;
		}
	}
	eval = 0;
	int is_correct ;
	int curSize, idx, nextSize, nextIdx, nextS, maxNumOfBlksAtCurPage;

	//2. Free some blocks WITHOUT freeing their pages
	cprintf_colored(TEXT_cyan,"\n2. Free some blocks WITHOUT freeing their pages [15%]\n");
	is_correct = 1;
	{
		//At each level that consume ONLY 1 page, free all its blocks (except 1)
		curSize = 1<<LOG2_MIN_SIZE ;
		maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
		idx = 0;
		int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
		nextSize = 0;
		nextIdx = 0;
		for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
		{
			//Skip removing the last block at the current size
			if (s == curSize)
			{
				//Reinitialize
				{
					curSize <<= 1;
					idx++ ;
					maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
				}
				//Stop the loop if the # allocation at current size exceed one page
				if (numOfAllocBlocksPerSize[idx] / maxNumOfBlksAtCurPage > 0)
				{
					nextSize = curSize;
					nextIdx = idx ;
					nextS = s + 1;
					break;
				}
				else
				{
					continue;
				}
			}

			//Free the current block
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
		}
		//Check # free frames (should not be changed)
		{
			freeFramesAfter = sys_calculate_free_frames();
			if (freeFramesAfter != freeFramesBefore)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #2: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",freeFramesBefore - freeFramesAfter , 0);
			}
		}
	}
	if (is_correct) eval += 15;

	//3. Allocate same blocks after free with diff. content (pages should NOT be allocated)
	cprintf_colored(TEXT_cyan,"\n3. Allocate same blocks after free with diff. content (pages should NOT be allocated) [15%]\n");
	is_correct = 1;
	{
		curSize = 1<<LOG2_MIN_SIZE ;
		idx = 0;
		void *va;
		int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
		for (int s = 1; s < nextS; ++s)
		{
			//Skip the last block at the current size
			if (s == curSize)
			{
				//Reinitialize
				{
					curSize <<= 1;
					idx++ ;
				}
				continue;
			}
			//allocate a new block with the curSize
			{
				va = alloc_block(s);
				startBlockVAs[s] = va;
				numOfAllocBlocksPerSize[idx]++ ;
			}
			//fill the entire block with different content
			for (int i = 0; i < curSize/2; ++i)
			{
				startBlockVAs[s][i] = s * 2;
			}
		}
		//Check # free frames (should not be changed)
		{
			freeFramesAfter = sys_calculate_free_frames();
			if (freeFramesAfter != freeFramesBefore)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #3: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",freeFramesBefore - freeFramesAfter , 0);
			}
		}
	}
	if (is_correct) eval += 15;

	//4. Free some blocks WITH freeing their pages
	cprintf_colored(TEXT_cyan,"\n4. Free some blocks WITH freeing their pages [15%]\n");
	int expectedNumOfRemovedPages = 0;
	is_correct = 1;
	{
		curSize = nextSize ;
		maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
		idx = nextIdx;
		int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
		for (int s = nextS; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
		{
			//Free the current block
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			//Update for next size
			if (s == curSize)
			{
				//Reinitialize
				curSize <<= 1;
				idx++ ;
				maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
				expectedNumOfRemovedPages++;
			}
			else if (numOfAllocBlocksPerSize[idx] % maxNumOfBlksAtCurPage == 0)
			{
				expectedNumOfRemovedPages++;
			}
		}
		//Check # free frames (should be changed)
		{
			freeFramesAfter = sys_calculate_free_frames();
			if (freeFramesAfter - freeFramesBefore != expectedNumOfRemovedPages)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #4: WRONG! number of free frames is not as expected. Actual: %d, Expected: %d\n",freeFramesAfter - freeFramesBefore, expectedNumOfRemovedPages);
			}
		}
	}
	if (is_correct) eval += 15;

	//5. Allocate same blocks after free with diff. content (pages should be allocated)
	cprintf_colored(TEXT_cyan,"\n5. Allocate same blocks after free with diff. content (pages should be allocated) [15%]\n");
	is_correct = 1;
	{
		curSize = nextSize ;
		maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
		idx = nextIdx;
		int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
		void* va;
		for (int s = nextS; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
		{
			//allocate a new block with the curSize
			{
				va = alloc_block(s);
				startBlockVAs[s] = va;
				numOfAllocBlocksPerSize[idx]++ ;
			}
			//fill the entire block with different content
			for (int i = 0; i < curSize/2; ++i)
			{
				startBlockVAs[s][i] = s * 2;
			}
			if (s == curSize)
			{
				//Reinitialize
				curSize <<= 1;
				idx++ ;
			}

		}
		//Check # free frames (should be changed)
		{
			freeFramesAfter = sys_calculate_free_frames();
			if (freeFramesBefore - freeFramesAfter != expectedNumOfRemovedPages)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #5: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n", freeFramesBefore - freeFramesAfter, expectedNumOfRemovedPages);
			}
		}
	}
	if (is_correct) eval += 15;

	//6. Check content of all blocks
	cprintf_colored(TEXT_cyan,"\n6. Check content of all blocks [20%]\n");
	is_correct = 1;
	{
		curSize = 1<<LOG2_MIN_SIZE ;
		idx = 0;
		void *va;
		int mult ;
		for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
		{
			//Only last block at the current size has its original values
			mult = s >= nextS? 2 : s == curSize ? 1 : 2;

			//check the content of the current block
			int sum = 0;
			for (int i = 0; i < curSize/2; ++i)
			{
				sum += startBlockVAs[s][i] ;
			}
			if (is_correct && sum != mult * s * curSize/2)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR,"Free Block Alloc #6: wrong content at block size %d. Expected = %d, Actual = %d\n", s, mult * s * curSize/2, sum);
				break;
			}

			if (s == curSize)
			{
				curSize <<= 1;
				idx++ ;
			}
		}
	}
	if (is_correct) eval += 20;

	//7. Free all blocks
	cprintf_colored(TEXT_cyan,"\n7. Free all blocks [20%]\n");
	is_correct = 1;
	{
		int freeFramesAfter = 0 ;
		int expectedNumOfRemovedPages = numOfAllocPages;
		for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
		{
			//Free the current block
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
		}
		//Check # free frames (should be restored to the original value before allocations)
		{
			freeFramesAfter = sys_calculate_free_frames();
			if (origFreeFrames - freeFramesAfter != 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #7: WRONG! number of free frames is not restored correctly. Actual: %d, Expected: %d\n",origFreeFrames - freeFramesAfter, 0);
			}
		}
		//Check free block lists
		{
			int expectedNumOfFreeBlocks8 = PAGE_SIZE / 8 - 1; //since initial alloc of 8B at boot-time
			int expectedNumOfFreeBlocks16 = PAGE_SIZE / 16 - 1; //since initial alloc of 16B at boot-time
			int expectedNumOfFreeBlocks ;
			for (int i = 0; i < numOfLevels; ++i)
			{
				expectedNumOfFreeBlocks = 0;
				if (i == 0) expectedNumOfFreeBlocks = expectedNumOfFreeBlocks8;
				if (i == 1) expectedNumOfFreeBlocks = expectedNumOfFreeBlocks16;

				if (LIST_SIZE(&freeBlockLists[i]) != expectedNumOfFreeBlocks)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #7: WRONG! number of free blocks at level %d is not correct. Actual: %d, Expected: %d\n", i, LIST_SIZE(&freeBlockLists[i]), expectedNumOfFreeBlocks);
				}
			}
		}
	}
	if (is_correct) eval += 20;

	cprintf_colored(TEXT_light_green,"\nTest kfree Block Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kfree_FF_both()
{
	panic("not implemented function");
}
int test_kfree_NF_both()
{
	panic("not implemented function");
}
int test_kfree_BF_both()
{
	panic("not implemented function");
}
int test_kfree_WF_both()
{
	panic("not implemented function");
}
int test_kfree_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/****************************** ADDRESS CONVERSION TEST AREA **********************************/
/**********************************************************************************************/
inline bool isVAInsideFreedAreas(uint32 va, uint32 *startOfFreedAreas, uint32 *endOfFreedAreas, int numOfFreedAreas)
{
	for (int i = 0; i < numOfFreedAreas; ++i)
	{
		if (va >= startOfFreedAreas[i] && va <= endOfFreedAreas[i])
			return 1;
	}
	return 0;
}
uint32 allPAs[(128*Mega)/PAGE_SIZE] ;

int test_kheap_phys_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces in both allocators
	int correct = 1;
	int eval;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in both allocators\n");
	{
		eval = initial_block_allocations();
		eval += initial_page_allocations();
		if (eval != 200)
		{
			cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
			return 0;
		}
	}
	eval = 0;
	//2. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc only
	cprintf_colored(TEXT_cyan,"\n2. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc only [20%]\n");
	correct = 1;
	{
		uint32 va;
		uint32 offset = 353;
		uint32 startVA = ACTUAL_PAGE_ALLOC_START + offset;
		uint32 endVA = ACTUAL_PAGE_ALLOC_START + totalRequestedSize;
		int i = 0;
		for (va = startVA; va < endVA; va+=PAGE_SIZE)
		{
			allPAs[i++] = kheap_physical_address(va);
		}
		int ii = i ;
		i = 0;
		int j;
		for (va = startVA; va < endVA; )
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("2.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			for (j = PTX(va); i < ii && j < 1024 && va < endVA; ++j, ++i)
			{
				if (((ptr_table[j] & 0xFFFFF000)+(va & 0x00000FFF))!= allPAs[i])
				{
					if (correct)
					{
						//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
						correct = 0;
						cprintf_colored(TEXT_TESTERR_CLR,"2.2 Wrong kheap_physical_address\n");
					}
				}
				va+=PAGE_SIZE;
			}
		}
	}
	if (correct)	eval+=20 ;

	//3. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc only
	cprintf_colored(TEXT_cyan,"\n3. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc only [20%]\n");
	correct = 1 ;
	{
		int i;
		uint32 va, pa;
		for (i = 1; i <= DYN_ALLOC_MAX_BLOCK_SIZE; i++)
		{
			va = (uint32)startBlockVAs[i];
			pa = kheap_physical_address(va);
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("3.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			if (((ptr_table[PTX(va)] & 0xFFFFF000)+(va & 0x00000FFF))!= pa)
			{
				//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
				if (correct)
				{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"3.2 Wrong kheap_physical_address\n"); }
			}
		}
	}
	if (correct)	eval+=20 ;

	//4. kfree some of the allocated spaces in both allocators
	cprintf_colored(TEXT_cyan,"\n4. kfree some of the allocated spaces in both allocators\n");
	uint32 startOfFreedAreas[3] = {0};
	uint32 endOfFreedAreas[3] = {0};
	uint32 startOfFreedBlocks[2] = {0};
	uint32 endOfFreedBlocks[2] = {0};

	{
		//[PAGE ALLOCATOR]
		{
			uint32 startVA = BLOCK_ALLOC_LIMIT + PAGE_SIZE;

			//kfree 1st 4 MB
			int freeFrames = sys_calculate_free_frames() ;
			int freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[0]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[0], PAGE_SIZE)/PAGE_SIZE ) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.1 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[0] = startVA ;
			endOfFreedAreas[0] = startOfFreedAreas[0] + ROUNDUP(requestedSizes[0], PAGE_SIZE) - 1;

			//kfree 1st 2 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[2]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[2], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[1] = startVA + ROUNDUP(requestedSizes[0], PAGE_SIZE) + ROUNDUP(requestedSizes[1], PAGE_SIZE) ;
			endOfFreedAreas[1] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE) - 1;

			//kfree 1st 1 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[4]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[4], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[2] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE)+ ROUNDUP(requestedSizes[3], PAGE_SIZE) ;
			endOfFreedAreas[2] = startOfFreedAreas[2] + ROUNDUP(requestedSizes[4], PAGE_SIZE) - 1 ;
		}
		//[BLOCK ALLOCATOR]
		int idx;
		{
			//kfree 8B blocks (except one block of boot-time)
			idx = 0;
			for (int s = 1; s <= 8; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 1);

			//kfree 64B blocks
			idx = 3;
			for (int s = 33; s <= 64; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 0);
			startOfFreedBlocks[0] = 33;
			endOfFreedBlocks[0] = 64;

			//kfree 256B blocks
			idx = 5;
			for (int s = 129; s <= 256; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 0);
			startOfFreedBlocks[1] = 129;
			endOfFreedBlocks[1] = 256;
		}
	}

	uint32 expected;
	//5. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc and kfree
	cprintf_colored(TEXT_cyan,"\n5. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc and kfree [25%]\n");
	correct = 1 ;
	{
		uint32 va;
		uint32 offset = 121;
		uint32 startVA = ACTUAL_PAGE_ALLOC_START+offset;
		uint32 endVA = ACTUAL_PAGE_ALLOC_START + totalRequestedSize;
		uint32 i = 0;
		for (va = startVA; va < endVA; va+=PAGE_SIZE)
		{
			allPAs[i++] = kheap_physical_address(va);
		}
		int ii = i ;
		i = 0;
		int j;
		for (va = startVA; va < endVA; )
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
				if (correct)
				{ correct = 0; panic("5.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			for (j = PTX(va); i < ii && j < 1024 && va < endVA; ++j, ++i)
			{
				if (isVAInsideFreedAreas(va, startOfFreedAreas, endOfFreedAreas, 3))
				{
					expected = 0 ;
				}
				else
				{
					expected = (ptr_table[j] & 0xFFFFF000) + (va & 0x00000FFF);
				}
				//if (((ptr_table[j] & 0xFFFFF000)+((ptr_table[j] & PERM_PRESENT) == 0? 0 : va & 0x00000FFF)) != allPAs[i])
				if (expected != allPAs[i])
				{
					//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
					if (correct)
					{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"5.2 Wrong kheap_physical_address\n"); }
				}
				va += PAGE_SIZE;
			}
		}
	}
	if (correct)	eval+=25 ;

	//6. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc and kfree
	cprintf_colored(TEXT_cyan,"\n6. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc and kfree [25%]\n");
	correct = 1 ;
	{
		uint32 va, pa;
		for (int i = 1; i <= DYN_ALLOC_MAX_BLOCK_SIZE; i++)
		{
			va = (uint32)startBlockVAs[i];
			pa = kheap_physical_address(va);
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("6.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			if (isVAInsideFreedAreas(i, startOfFreedBlocks, endOfFreedBlocks, 2))
			{
				expected = 0 ;
			}
			else
			{
				expected = (ptr_table[PTX(va)] & 0xFFFFF000) + (va & 0x00000FFF);
			}
			if (expected != pa)
			{
				if (correct)
				{
					cprintf_colored(TEXT_TESTERR_CLR,"\nBlock Size = %d, VA = %x, table entry = %x\nkhep_pa = %x, expected = %x\n",i, va, (ptr_table[PTX(va)] & 0xFFFFF000) , pa, expected);
					correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR,"6.2 Wrong kheap_physical_address\n");
				}
			}
		}
	}
	if (correct)	eval+=25 ;

	//7. test kheap_physical_address on non-mapped area
	cprintf_colored(TEXT_cyan,"\n7. test kheap_physical_address on non-mapped area [10%]\n");
	correct = 1 ;
	{
		uint32 va;
		uint32 startVA = ACTUAL_PAGE_ALLOC_START +totalRequestedSize;
		int ii = (KERNEL_HEAP_MAX - startVA) / PAGE_SIZE;
		uint32 i = 0;
		int j;
		long long va2;
		for (va2 = startVA; va2 < (long long)KERNEL_HEAP_MAX; va2+=PTSIZE)
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, (uint32)va2, &ptr_table);
			if (ptr_table == NULL)
			{
				if (correct)
				{ correct = 0; panic("7.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }
			}
			for (j = 0; i < ii && j < 1024; ++j, ++i)
			{
				//if ((ptr_table[j] & 0xFFFFF000) != allPAs[i])
				unsigned int page_va = startVA+i*PAGE_SIZE;
				unsigned int supposed_kheap_phys_add = kheap_physical_address(page_va);
				//if (((ptr_table[j] & 0xFFFFF000)+((ptr_table[j] & PERM_PRESENT) == 0? 0 : page_va & 0x00000FFF)) != supposed_kheap_phys_add)
				if (supposed_kheap_phys_add != 0)
				{
					//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va2 + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
					if (correct)
					{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Wrong kheap_physical_address\n"); }
				}
			}
		}
	}
	if (correct)	eval+=10 ;

	cprintf_colored(TEXT_light_green,"\ntest kheap_physical_address completed. Eval = %d%\n", eval);

	return 1;

}

int test_kheap_virt_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/

	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces in both allocators
	int correct = 1;
	int eval;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in both allocators\n");
	{
		eval = initial_block_allocations();
		eval += initial_page_allocations();

		if (eval != 200)
		{
			cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
			return 0;
		}
	}
	eval = 0;

	//2. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc only
	cprintf_colored(TEXT_cyan,"\n2. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc only [20%]\n");
	int numOfFrames = totalRequestedSize/PAGE_SIZE ;
	correct = 1;
	{
		uint32 va;
		uint32 startVA = ACTUAL_PAGE_ALLOC_START ;
		uint32 endVA = ACTUAL_PAGE_ALLOC_START + totalRequestedSize;
		int i = 0;
		int j;
		for (va = startVA; va < endVA; )
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("2.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			for (j = PTX(va); i < numOfFrames && j < 1024 && va < endVA; ++j, ++i)
			{
				uint32 offset = j;
				assert((ptr_table[j] & PERM_PRESENT) == PERM_PRESENT);
				allPAs[i] = (ptr_table[j] & 0xFFFFF000) + offset;
				uint32 retrievedVA = kheap_virtual_address(allPAs[i]);
				//cprintf_colored(TEXT_TESTERR_CLR,"va to check = %x\n", va);
				uint32 expectedVA = (va+offset);
				if (retrievedVA != expectedVA)
				{
					correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR,"2.2 Wrong kheap_virtual_address. Expected = %x, Actual = %x\n", expectedVA, retrievedVA);
				}
				va+=PAGE_SIZE;
			}
		}
	}
	if (correct)	eval+=20 ;

	//3. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc only
	cprintf_colored(TEXT_cyan,"\n3. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc only [20%]\n");
	correct = 1 ;
	{
		uint32 va, pa;
		for (int i = 1; i <= DYN_ALLOC_MAX_BLOCK_SIZE; i++)
		{
			va = (uint32)startBlockVAs[i];
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("3.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			assert((ptr_table[PTX(va)] & PERM_PRESENT) == PERM_PRESENT);
			pa = (ptr_table[PTX(va)] & 0xFFFFF000)+(va & 0x00000FFF);
			uint32 retrievedVA = kheap_virtual_address(pa);
			uint32 expectedVA = va;
			//cprintf_colored(TEXT_TESTERR_CLR,"va to check = %x\n", va);
			if (retrievedVA != expectedVA)
			{
				if (correct)
				{
					correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR,"3.2 Wrong kheap_virtual_address at block size %d! Expected = %x, Actual = %x\n", i, expectedVA, retrievedVA);
				}
			}
		}
	}
	if (correct)	eval+=20 ;

	//4. kfree some of the allocated spaces in both allocators
	cprintf_colored(TEXT_cyan,"\n4. kfree some of the allocated spaces in both allocators\n");
	uint32 startOfFreedAreas[3] = {0};
	uint32 endOfFreedAreas[3] = {0};
	uint32 startOfFreedBlocks[2] = {0};
	uint32 endOfFreedBlocks[2] = {0};
	{
		//[PAGE ALLOCATOR]
		{
			uint32 startVA = BLOCK_ALLOC_LIMIT + PAGE_SIZE;

			//kfree 1st 4 MB
			int freeFrames = sys_calculate_free_frames() ;
			int freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[0]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[0], PAGE_SIZE)/PAGE_SIZE ) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.1 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[0] = startVA ;
			endOfFreedAreas[0] = startOfFreedAreas[0] + ROUNDUP(requestedSizes[0], PAGE_SIZE) - 1;

			//kfree 1st 2 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[2]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[2], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[1] = startVA + ROUNDUP(requestedSizes[0], PAGE_SIZE) + ROUNDUP(requestedSizes[1], PAGE_SIZE) ;
			endOfFreedAreas[1] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE) - 1;

			//kfree 1st 1 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[4]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[4], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[2] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE)+ ROUNDUP(requestedSizes[3], PAGE_SIZE) ;
			endOfFreedAreas[2] = startOfFreedAreas[2] + ROUNDUP(requestedSizes[4], PAGE_SIZE) - 1 ;
		}
		//[BLOCK ALLOCATOR]
		int idx;
		{
			//kfree 8B blocks (except one block of boot-time)
			idx = 0;
			for (int s = 1; s <= 8; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 1);

			//kfree 64B blocks
			idx = 3;
			for (int s = 33; s <= 64; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 0);
			startOfFreedBlocks[0] = 33;
			endOfFreedBlocks[0] = 64;

			//kfree 256B blocks
			idx = 5;
			for (int s = 129; s <= 256; ++s)
			{
				free_block(startBlockVAs[s]);
				numOfAllocBlocksPerSize[idx]--;
			}
			assert(numOfAllocBlocksPerSize[idx] == 0);
			startOfFreedBlocks[1] = 129;
			endOfFreedBlocks[1] = 256;
		}
	}

	//5. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc and kfree
	cprintf_colored(TEXT_cyan,"\n5. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc and kfree [25%]\n");
	correct = 1 ;
	{
		uint32 va;
		uint32 startVA = ACTUAL_PAGE_ALLOC_START;
		uint32 endVA = ACTUAL_PAGE_ALLOC_START + totalRequestedSize;
		uint32 i = 0;

		for (i = 0 ; i < numOfFrames; ++i)
		{
			uint32 retrievedVA = kheap_virtual_address(allPAs[i]);
			uint32 expectedVA ;
			if (isVAInsideFreedAreas(startVA, startOfFreedAreas, endOfFreedAreas, 3))
			{
				expectedVA = 0;
			}
			else
			{
				expectedVA = (startVA + (allPAs[i] & 0xFFF));
			}
			if (retrievedVA != expectedVA)
			{
				if (correct)
				{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"5.1 Wrong kheap_virtual_address. Expected = %x, Actual = %x\n", expectedVA, retrievedVA); }
			}
			startVA += PAGE_SIZE;
		}
	}
	if (correct)	eval+=25 ;

	//6. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc and kfree
	cprintf_colored(TEXT_cyan,"\n6. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc and kfree [25%]\n");
	correct = 1 ;
	{
		uint32 va, pa;
		for (int i = 1; i <= DYN_ALLOC_MAX_BLOCK_SIZE; i++)
		{
			va = (uint32)startBlockVAs[i];

			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, va, &ptr_table);
			if (ptr_table == NULL)
			{ correct = 0; panic("6.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

			pa = (ptr_table[PTX(va)] & 0xFFFFF000) + (va & 0xFFF);

			uint32 retrievedVA = kheap_virtual_address(pa);

			uint32 expectedVA;
			if (isVAInsideFreedAreas(i, startOfFreedBlocks, endOfFreedBlocks, 2))
			{
				expectedVA = 0 ;
			}
			else
			{
				expectedVA = va;
			}
			if (expectedVA != retrievedVA)
			{
				if (correct)
				{
					correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR,"6.2 Wrong kheap_virtual_address at block size %d! Expected = %x, Actual = %x\n", i, expectedVA, retrievedVA);
				}
			}
		}
	}
	if (correct)	eval+=25 ;

	correct = 1 ;
	//7. test kheap_virtual_address on frames of KERNEL CODE
	cprintf_colored(TEXT_cyan,"\n6. test kheap_virtual_address on frames of KERNEL CODE [10%]\n");
	{
		uint32 i;
		for (i = 1*Mega; i < (uint32)(end_of_kernel - KERNEL_BASE); i+=PAGE_SIZE)
		{
			uint32 retrievedVA = kheap_virtual_address(i);
			if (retrievedVA != 0)
			{
				if (correct)
				{
					cprintf_colored(TEXT_TESTERR_CLR,"\nPA = %x, retrievedVA = %x\n", i, retrievedVA);
					correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.1 Wrong kheap_virtual_address\n");
				}
			}
		}
	}
	if (correct)	eval+=10 ;

	cprintf_colored(TEXT_light_green,"\ntest kheap_virtual_address completed. Eval = %d%\n", eval);

	return 1;

}


/**********************************************************************************************/
/********************************** KREALLOC TESTING AREA *************************************/
/**********************************************************************************************/
int test_krealloc_FF_page()
{
	panic("not implemented function");
}
int test_krealloc_NF_page()
{
	panic("not implemented function");
}
int test_krealloc_BF_page()
{
	panic("not implemented function");
}
int test_krealloc_WF_page()
{
	panic("not implemented function");
}
int test_krealloc_CF_page()
{
	panic("not implemented function");
}

int test_krealloc_FF_block()
{
	panic("not implemented function");
}
int test_krealloc_NF_block()
{
	panic("not implemented function");
}
int test_krealloc_BF_block()
{
	panic("not implemented function");
}
int test_krealloc_WF_block()
{
	panic("not implemented function");
}
int test_krealloc_CF_block()
{
	panic("not implemented function");
}

int test_krealloc_FF_both()
{
	panic("not implemented function");
}
int test_krealloc_NF_both()
{
	panic("not implemented function");
}
int test_krealloc_BF_both()
{
	panic("not implemented function");
}
int test_krealloc_WF_both()
{
	panic("not implemented function");
}
int test_krealloc_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/*************************** FAST PAGE ALLOCATOR TESTING AREA *********************************/
/**********************************************************************************************/
int test_fast_FF()
{
	panic("not implemented function");
}
int test_fast_NF()
{
	panic("not implemented function");
}
int test_fast_BF()
{
	panic("not implemented function");
}
int test_fast_WF()
{
	panic("not implemented function");
}
int test_fast_CF()
{
	panic("not implemented function");
}




/**********************************************************************************************/
/******************************** OLD IMPLEMENTATION AREA *************************************/
/**********************************************************************************************/

int initFreeFrames;
int initFreeDiskFrames ;
uint8 firstCall = 1 ;
int test_three_creation_functions()
{
	if (firstCall)
	{
		firstCall = 0;
		initFreeFrames = sys_calculate_free_frames() ;
		initFreeDiskFrames = pf_calculate_free_frames() ;
		//Run simple user program
		{
			char command[100] = "run fos_add 4096";
			execute_command(command) ;
		}
	}
	//Ensure that the user directory, page WS and page tables are allocated in KERNEL HEAP
	{
		struct Env * e = NULL;
		struct Env * ptr_env = NULL;
		LIST_FOREACH(ptr_env, &ProcessQueues.env_exit_queue)
		{
			if (strcmp(ptr_env->prog_name, "fos_add") == 0)
			{
				e = ptr_env ;
				break;
			}
		}
		if (e->pageFaultsCounter != 0)
			panic("Page fault is occur while not expected to. Review the three creation functions");

#if USE_KHEAP
		int pagesInWS = LIST_SIZE(&(e->page_WS_list));
#else
		int pagesInWS = env_page_ws_get_size(e);
#endif
		int curFreeFrames = sys_calculate_free_frames() ;
		int curFreeDiskFrames = pf_calculate_free_frames() ;
		//cprintf_colored(TEXT_TESTERR_CLR,"\ndiff in page file = %d, pages in WS = %d\n", initFreeDiskFrames - curFreeDiskFrames, pagesInWS);
		if ((initFreeDiskFrames - curFreeDiskFrames) != pagesInWS) panic("Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)");
		//cprintf_colored(TEXT_TESTERR_CLR,"\ndiff in mem frames = %d, pages in WS = %d\n", initFreeFrames - curFreeFrames, pagesInWS);
		if ((initFreeFrames - curFreeFrames) != 12/*WS*/ + 2*1/*DIR*/ + 2*3/*Tables*/ + 1 /*user WS table*/ + pagesInWS) panic("Wrong allocation: pages are not loaded successfully into memory");

		//allocate 4 KB
		char *ptr = kmalloc(4*kilo);
		if ((uint32) ptr !=  (ACTUAL_PAGE_ALLOC_START + (12+2*1+2*3+1)*PAGE_SIZE)) panic("Wrong start address for the allocated space... make sure you create the dir, table and page WS in KERNEL HEAP");
	}

	cprintf_colored(TEXT_TESTERR_CLR,"\nCongratulations!! test the 3 creation functions is completed successfully.\n");

	return 1;
}



extern void kfreeall() ;

int test_kfreeall()
{
	panic("not implemented function");
}


extern void kexpand(uint32 newSize) ;

int test_kexpand()
{
	panic("not implemented function");
}

extern void kshrink(uint32 newSize) ;

int test_kshrink()
{
	panic("not implemented function");
}


int test_kfreelast()
{
	panic("not implemented function");
}

