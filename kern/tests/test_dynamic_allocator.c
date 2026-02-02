/*
 * test_lists_managment.c

 *
 *  Created on: Oct 6, 2022
 *  Updated on: Sept 20, 2023
 *      Author: HP
 */
#include <inc/queue.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/dynamic_allocator.h>
#include <inc/memlayout.h>

//NOTE: ALL tests in this file shall work with USE_KHEAP = 0

/***********************************************************************************************************************/
#define Mega  (1024*1024)
#define kilo (1024)
#define numOfLevels (LOG2_MAX_SIZE - LOG2_MIN_SIZE + 1)

short* startVAsInit[DYN_ALLOC_MAX_BLOCK_SIZE + 1] ;
short* endVAsInit[DYN_ALLOC_MAX_BLOCK_SIZE + 1] ;

__inline__ uint8 IDX(uint32 size)
{
	size>>= LOG2_MIN_SIZE;
	int index = 0;
	while ((size>>=1) != 0)
	{
		index++;
	}
	return index;
}

int check_dynalloc_datastruct(uint32 curSize, uint32 numOfBlksAtCurSize)
{
	int maxNumOfBlksPerPage = PAGE_SIZE / curSize;
	int expectedNumOfCompletePages = numOfBlksAtCurSize / maxNumOfBlksPerPage;
	int expectedNumOfInCompletePages = numOfBlksAtCurSize % maxNumOfBlksPerPage != 0? 1 : 0;
	int expectedNumOfFreeBlks = expectedNumOfInCompletePages * (maxNumOfBlksPerPage - numOfBlksAtCurSize % maxNumOfBlksPerPage);

	//[1] Check PageBlkInfoArr
	int numOfPages = DYN_ALLOC_MAX_SIZE / PAGE_SIZE;
	int actualNumOfCompletePages = 0;
	int actualNumOfInCompletePages = 0;
	int actualNumOfFreeBlks = 0;
	for (int i = 0; i < numOfPages; ++i)
	{
		if (pageBlockInfoArr[i].block_size == curSize)
		{
			if (pageBlockInfoArr[i].num_of_free_blocks == 0)
			{
				actualNumOfCompletePages++;
			}
			else
			{
				actualNumOfInCompletePages++;
				actualNumOfFreeBlks += pageBlockInfoArr[i].num_of_free_blocks;
			}
		}
	}
	if (actualNumOfCompletePages != expectedNumOfCompletePages ||
			actualNumOfInCompletePages != expectedNumOfInCompletePages ||
			actualNumOfFreeBlks != expectedNumOfFreeBlks)
	{
		cprintf_colored(TEXT_TESTERR_CLR, "PageBlkInfoArr is not set/updated correctly!\n");
//		cprintf_colored(TEXT_cyan, "actualNumOfCompletePages = %d, expectedNumOfCompletePages = %d\n", actualNumOfCompletePages, expectedNumOfCompletePages);
//		cprintf_colored(TEXT_cyan, "actualNumOfInCompletePages = %d, expectedNumOfInCompletePages = %d\n", actualNumOfInCompletePages, expectedNumOfInCompletePages);
//		cprintf_colored(TEXT_cyan, "actualNumOfFreeBlks = %d, expectedNumOfFreeBlks = %d\n", actualNumOfFreeBlks, expectedNumOfFreeBlks);
		return 0;
	}

	//[2] Check freeBlkLists
	int index = IDX(curSize);
	struct BlockElement_List *ptrList = &freeBlockLists[index];
	int n = 0;
	struct BlockElement *ptrBlk;
	LIST_FOREACH(ptrBlk, ptrList)
	{
		n++;
	}
	if (LIST_SIZE(ptrList) != expectedNumOfFreeBlks || n != expectedNumOfFreeBlks)
	{
		cprintf_colored(TEXT_TESTERR_CLR,"freeBlockLists[%d] is not updated correctly!", index);
		return 0;
	}
	return 1;
}


extern uint32* ptr_page_directory;
extern void unmap_frame(uint32 *ptr_page_directory, uint32 virtual_address);
extern uint32 sys_calculate_free_frames() ;

void remove_current_mappings(uint32 startVA, uint32 endVA)
{
	assert(startVA >= KERNEL_HEAP_START && endVA >= KERNEL_HEAP_START);
	for (uint32 va = startVA; va < endVA; va+=PAGE_SIZE)
	{
		unmap_frame(ptr_page_directory, va);
	}
}
/***********************************************************************************************************************/

void test_initialize_dynamic_allocator()
{
#if USE_KHEAP
	panic("test_initialize_dynamic_allocator: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return ;
#endif

	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run ANYTHING before or after it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");

	initialize_dynamic_allocator(KERNEL_HEAP_START - DYN_ALLOC_MAX_SIZE, KERNEL_HEAP_START);

	//Check#1: Limits
	cprintf_colored(TEXT_cyan, "\nCheck#1: Limits \n");
	if (dynAllocStart != KERNEL_HEAP_START  - DYN_ALLOC_MAX_SIZE || dynAllocEnd != KERNEL_HEAP_START )
	{
		panic("DA limits are not set correctly");
	}
	//Check#2: PageBlockInfoArr
	cprintf_colored(TEXT_cyan, "\nCheck#2: PageBlockInfoArr \n");
	int numOfPages = DYN_ALLOC_MAX_SIZE / PAGE_SIZE;
	for (int i = 0; i < numOfPages; ++i)
	{
		if (pageBlockInfoArr[i].block_size || pageBlockInfoArr[i].num_of_free_blocks)
		{
			panic("DA pageBlockInfoArr are not initialized correctly");
		}
	}
	//Check#3: freePagesList
	cprintf_colored(TEXT_cyan, "\nCheck#3: freePagesList \n");
	int n = 0;
	struct PageInfoElement *ptrPI;
	LIST_FOREACH(ptrPI, &freePagesList)
	{
		if (ptrPI != &pageBlockInfoArr[n])
		{
			panic("DA freePagesList is not initialized correctly! Make sure that the pages are added to the list in their correct order");
		}
		n++;
	}
	if (LIST_SIZE(&freePagesList) != numOfPages || n != numOfPages)
	{
		panic("DA freePagesList is not initialized correctly! one or more pages are not added correctly");
	}
	//Check#4: freeBlockLists
	cprintf_colored(TEXT_cyan, "\nCheck#4: freeBlockLists \n");
	int numOfSizes = LOG2_MAX_SIZE - LOG2_MIN_SIZE + 1;
	for (int i = 0; i < numOfSizes; ++i)
	{
		struct BlockElement_List *ptrList = &freeBlockLists[i];
		if (LIST_SIZE(ptrList) || LIST_FIRST(ptrList) || LIST_LAST(ptrList))
		{
			panic("DA freeBlockLists[%d] is not initialized correctly!", i);
		}
	}

	cprintf_colored(TEXT_light_green,
			"============================================================================="
			"\nCongratulations!! test initialize_dynamic_allocator completed successfully.\n"
			"=============================================================================\n");
}

int test_initial_alloc()
{
#if USE_KHEAP
	panic("test_initial_alloc: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return 0;
#endif

	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run ANYTHING before or after it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");

	//Remove the current 1-to-1 mapping of the KERNEL HEAP area since the USE_KHEAP = 0 for this test
	uint32 startDA = KERNEL_HEAP_START ;
	uint32 sizeDA = 0x2AE000 ;
	uint32 endDA = KERNEL_HEAP_START + sizeDA ;
	remove_current_mappings(startDA, endDA);
	initialize_dynamic_allocator(startDA, endDA);

	int eval = 0;
	bool is_correct = 1;

	int freeFramesBefore = sys_calculate_free_frames();
	void *va ;
	//====================================================================//
	/*INITIAL ALLOC Scenario 1: Allocate set of blocks for each possible block size (consume entire space)*/
	cprintf_colored(TEXT_cyan, "\n1: Allocate set of blocks for each possible block size (consume entire space)\n\n") ;
	int curSize = 1<<LOG2_MIN_SIZE ;
	int numOfBlksAtCurSize = 0;
	int maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
	uint32 expectedVA = KERNEL_HEAP_START;
	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		va = alloc_block(s);
		startVAsInit[s] = va; *startVAsInit[s] = s ;
		endVAsInit[s] = va + curSize - sizeof(short); *endVAsInit[s] = s ;

		numOfBlksAtCurSize++;
		if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#1.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", s, expectedVA, va);
		}
		if (s == curSize)
		{
			//apply the following check only on the 1st three levels
			if (curSize <= 32)
			{
				if (is_correct)	eval += 5;
				is_correct = 1;
			}
			if (check_dynalloc_datastruct(curSize, numOfBlksAtCurSize) == 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#2.%d: WRONG! DA data structures are not correct\n", s);
			}
			if (is_correct)	eval += 5;
			//Reinitialize
			{
				curSize <<= 1;
				expectedVA += PAGE_SIZE;
				numOfBlksAtCurSize = 0;
				maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
				is_correct = 1;
			}
		}
		else if (numOfBlksAtCurSize % maxNumOfBlksAtCurPage == 0)
		{
			expectedVA += PAGE_SIZE;
		}

	}

	//====================================================================//
	/*INITIAL ALLOC Scenario 2: Allocate blocks of same size that consume remaining free blocks at all levels*/
	cprintf_colored(TEXT_cyan, "\n2: Allocate blocks of same size that consume remaining free blocks at all levels\n\n") ;
	is_correct = 1;
	//calculate expected number of free blocks at all levels
	int size1 = 0;
	int size2 = DYN_ALLOC_MIN_BLOCK_SIZE;
	int numOfRemFreeBlks[numOfLevels];
	uint32 expectedPageIndex[numOfLevels];
	uint32 prevAllocPages = 0;
	int idx = 0;
	while (size1 < DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		int numOfAllocBlks = size2 - size1 ;
		int expectedNumOfBlksPerPage = PAGE_SIZE / size2;
		if (numOfAllocBlks % expectedNumOfBlksPerPage != 0)
		{
			numOfRemFreeBlks[idx] = expectedNumOfBlksPerPage - (numOfAllocBlks % expectedNumOfBlksPerPage) ;
			expectedPageIndex[idx] = (prevAllocPages + numOfAllocBlks / expectedNumOfBlksPerPage) ;
			prevAllocPages += numOfAllocBlks / expectedNumOfBlksPerPage + 1;
		}
		else
		{
			numOfRemFreeBlks[idx] = 0;
			expectedPageIndex[idx] = 0;
			prevAllocPages += numOfAllocBlks / expectedNumOfBlksPerPage;
		}
		size1 = size2 ;
		size2 *= 2 ;
		idx++;
	}

	//Allocate a number of blocks of same size to consume all the remaining free blocks
	int blkSize = 1<<LOG2_MIN_SIZE ;
	for (int i = 0; i < numOfLevels; ++i)
	{
		uint32 expectedVA = KERNEL_HEAP_START + expectedPageIndex[i] * PAGE_SIZE;
		is_correct = 1;
		//cprintf_colored(TEXT_cyan,  "Level#%d: num of remaining free blocks = %d\n", i, numOfRemFreeBlks[i]);
		for (int j = 0; j < numOfRemFreeBlks[i]; ++j)
		{
			va = alloc_block(blkSize);
			int *tmpVal = va ;
			*tmpVal = 353 ;
			if (ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#3: WRONG! VA is not correct (i = %d, j = %d)\n", i, j);
				break;
			}
			if (*tmpVal != 353)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#4: wrong stored value in the allocated block\n");
			}
		}

		if (numOfRemFreeBlks[i] > 0)
		{
			if (LIST_SIZE(&freeBlockLists[i]) != 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#5: WRONG! there's still free blocks at level %d while not expected to\n", i);
			}
			if (pageBlockInfoArr[expectedPageIndex[i]].num_of_free_blocks != 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "alloc_block test#6: WRONG! there's still free blocks at page %d while not expected to\n", expectedPageIndex[i]);
			}
			if (is_correct)	eval += 5;
		}
	}

	//====================================================================//
	/*INITIAL ALLOC Scenario 3: Check stored data inside each allocated block*/
	cprintf_colored(TEXT_cyan, "\n3: Check stored data inside each allocated block\n\n") ;
	is_correct = 1;

	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		if (*(startVAsInit[s]) != s || *(endVAsInit[s]) != s)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "alloc_block #7.%d: WRONG! content of the block is not correct. Expected %d\n",s, s);
			break;
		}
	}
	if (is_correct)
	{
		eval += 10;
	}
	//====================================================================//
	/*INITIAL ALLOC Scenario 4: Check allocated frames*/
	cprintf_colored(TEXT_cyan, "\n4: Check allocated frames\n\n") ;
	is_correct = 1;
	int freeFramesAfter = sys_calculate_free_frames();
	int expectedNumOfAllocPages = 686;
	if (freeFramesBefore - freeFramesAfter != expectedNumOfAllocPages)
	{
		is_correct = 0;
		cprintf_colored(TEXT_TESTERR_CLR, "alloc_block #8: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",freeFramesBefore - freeFramesAfter , expectedNumOfAllocPages);
	}
	if (is_correct)
	{
		eval += 10;
	}

	return eval;
}

void test_alloc_block()
{
#if USE_KHEAP
	panic("test_alloc_block: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return;
#endif

	int eval = 0;
	bool is_correct;
	void* va = NULL;
	uint32 actualSize = 0;

	eval = test_initial_alloc();

	cprintf_colored(TEXT_light_green, "test alloc_block Evaluation = %d%\n", eval);
	return ;
}

void test_free_block()
{
#if USE_KHEAP
	panic("test_free_block: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return;
#endif

	cprintf_colored(TEXT_light_cyan, "===========================================================\n") ;
	cprintf_colored(TEXT_light_cyan, "NOTE: THIS TEST IS DEPEND ON BOTH ALLOCATE & FREE FUNCTIONS\n") ;
	cprintf_colored(TEXT_light_cyan, "===========================================================\n") ;

	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run ANYTHING before or after it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");

	void*expected_va ;

	//Remove the current 1-to-1 mapping of the KERNEL HEAP area since the USE_KHEAP = 0 for this test
	uint32 startDA = KERNEL_HEAP_START ;
	uint32 sizeDA = 0x2AE000 ;
	uint32 endDA = KERNEL_HEAP_START + sizeDA ;
	remove_current_mappings(startDA, endDA);
	initialize_dynamic_allocator(startDA, endDA);

	int eval = 0;
	bool is_correct = 1;

	int initialFreeFrames = sys_calculate_free_frames();
	void *va ;
	//====================================================================//
	/*INITIAL ALLOCATION: Allocate set of blocks for each possible block size (consume entire space)*/
	cprintf_colored(TEXT_cyan, "\n1: Allocate set of blocks for each possible block size (consume entire space)\n\n") ;
	int curSize = 1<<LOG2_MIN_SIZE ;
	int numOfBlksAtCurSize = 0;
	int maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
	uint32 expectedVA = KERNEL_HEAP_START;
	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		va = alloc_block(s);
		startVAsInit[s] = va; *startVAsInit[s] = s ;
		endVAsInit[s] = va + curSize - sizeof(short); *endVAsInit[s] = s ;

		numOfBlksAtCurSize++;
		if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
		{
			panic("free_block test#1.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", s, expectedVA, va);
		}
		if (s == curSize)
		{
			if (check_dynalloc_datastruct(curSize, numOfBlksAtCurSize) == 0)
			{
				panic("free_block test#2.%d: WRONG! DA data structures are not correct\n", s);
			}
			//Reinitialize
			{
				curSize <<= 1;
				expectedVA += PAGE_SIZE;
				numOfBlksAtCurSize = 0;
				maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
			}
		}
		else if (numOfBlksAtCurSize % maxNumOfBlksAtCurPage == 0)
		{
			expectedVA += PAGE_SIZE;
		}
	}

	//====================================================================//
	/*FREE: Remove all blocks (except 1) for each possible block size that consume at most 1 page (Page will nor be freed)*/
	cprintf_colored(TEXT_cyan, "\n2: Remove all blocks (except 1) for each block size that consume 1 page [30%]"
			"(Page will not be freed)\n\n") ;
	//calculate expected number of allocated & free blocks at all levels
	int size1 = 0;
	int size2 = DYN_ALLOC_MIN_BLOCK_SIZE;
	int numOfRemFreeBlks[numOfLevels];
	int numOfAllocBlks[numOfLevels];
	uint32 expectedPageIndex[numOfLevels];
	uint32 prevAllocPages = 0;
	int idx = 0;
	while (size1 < DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		numOfAllocBlks[idx] = size2 - size1;
		int expectedNumOfBlksPerPage = PAGE_SIZE / size2;
		if (numOfAllocBlks[idx] % expectedNumOfBlksPerPage != 0)
		{
			numOfRemFreeBlks[idx] = expectedNumOfBlksPerPage - (numOfAllocBlks[idx] % expectedNumOfBlksPerPage) ;
			expectedPageIndex[idx] = (prevAllocPages + numOfAllocBlks[idx] / expectedNumOfBlksPerPage) ;
			prevAllocPages += numOfAllocBlks[idx] / expectedNumOfBlksPerPage + 1;
		}
		else
		{
			numOfRemFreeBlks[idx] = 0;
			expectedPageIndex[idx] = 0;
			prevAllocPages += numOfAllocBlks[idx] / expectedNumOfBlksPerPage;
		}
		size1 = size2 ;
		size2 *= 2 ;
		idx++;
	}

	//At each level that consume ONLY 1 page, free all its blocks (except 1)
	curSize = 1<<LOG2_MIN_SIZE ;
	maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
	idx = 0;
	int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
	int nextSize = 0;
	int nextIdx = 0;
	is_correct = 1;
	for (int s = 1; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		//Skip removing the last block at the current size
		if (s == curSize)
		{
			if (is_correct) eval += 5;
			is_correct = 1;
			//Check the entire data structures
			if (check_dynalloc_datastruct(curSize, numOfAllocBlks[idx]) == 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR,"free_block test#4.%d: WRONG! DA data structures are not correct\n", s);
			}
			if (is_correct) eval += 5;
			is_correct = 1;
			//Reinitialize
			{
				curSize <<= 1;
				idx++ ;
				maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
			}
			//Stop the loop if the # allocation at current size exceed one page
			if (numOfAllocBlks[idx] / maxNumOfBlksAtCurPage > 0)
			{
				nextSize = curSize;
				nextIdx = idx ;
				break;
			}
			else
			{
				continue;
			}
		}

		free_block(startVAsInit[s]);

		numOfAllocBlks[idx]--;

		freeFramesAfter = sys_calculate_free_frames();
		//Check # free frames (should not be changed)
		if (is_correct && freeFramesAfter != freeFramesBefore)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "free_block test#3.%d: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",s, freeFramesBefore - freeFramesAfter , 0);
		}
	}

	//====================================================================//
	/*FREE: Remove remaining block in each block size that consume at most 1 page (Page should be freed)*/
	cprintf_colored(TEXT_cyan, "\n3: Remove remaining block in each block size that consume at most 1 page  [30%]"
			"(Page should be freed)\n") ;
	//At each level that consume ONLY 1 page, free its remaining block
	curSize = 1<<LOG2_MIN_SIZE ;
	idx = 0;
	while (curSize < nextSize)
	{
		//Check content of the block before removing it
		is_correct = 1;
		if (*startVAsInit[curSize] != curSize || *endVAsInit[curSize] != curSize)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "free_block test#4.%d: WRONG! block content is changed while it's not expected to.", curSize);
		}
		//free the remaining block at the current size
		freeFramesBefore = sys_calculate_free_frames();
		{
			free_block(startVAsInit[curSize]);

			numOfAllocBlks[idx]--;

			assert(numOfAllocBlks[idx] == 0);
		}

		//Check # free frames (should be increased by 1)
		freeFramesAfter = sys_calculate_free_frames();
		if (freeFramesAfter - freeFramesBefore != 1)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "free_block test#5.%d: WRONG! number of free frames is not as expected. Actual: %d, Expected: %d\n",curSize, freeFramesAfter - freeFramesBefore, 1);
		}
		if (is_correct) eval += 5;
		//Check the entire data structures
		is_correct = 1;
		if (check_dynalloc_datastruct(curSize, numOfBlksAtCurSize) == 0)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR,"free_block test#5.%d: WRONG! DA data structures are not correct\n", curSize);
		}
		if (is_correct) eval += 5;
		is_correct = 1;

		//Move to next block size
		{
			curSize <<= 1;
			idx++ ;
		}
	}
	//rescale to 60% instead of 80%
	eval = eval * 60 / 80;


	//====================================================================//
	/*FREE: Remove all blocks for each of the remaining block sizes*/
	cprintf_colored(TEXT_cyan, "\n4: Remove all blocks for each of the remaining block sizes  [20%]\n") ;
	curSize = nextSize ;
	int startSize = (nextSize>>1) + 1;
	idx = nextIdx;
	is_correct = 1;
	for (int s = startSize; s <= DYN_ALLOC_MAX_BLOCK_SIZE; ++s)
	{
		free_block(startVAsInit[s]);

		numOfAllocBlks[idx]--;

		//Skip removing the last block at the current size
		if (s == curSize)
		{
			assert(numOfAllocBlks[idx] == 0);

			//Check the entire data structures
			if (check_dynalloc_datastruct(curSize, numOfAllocBlks[idx]) == 0)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR,"free_block test#6.%d: WRONG! DA data structures are not correct\n", s);
				break;
			}
			//Reinitialize
			{
				curSize <<= 1;
				idx++ ;
			}
		}

	}

	//Check # free frames (should be returned to the original number (no tables are allocated in the DA range ))
	freeFramesAfter = sys_calculate_free_frames();
	if (freeFramesAfter != initialFreeFrames)
	{
		is_correct = 0;
		cprintf_colored(TEXT_TESTERR_CLR, "free_block test#7: WRONG! number of free frames is not returned to the original. Actual: %d, Original: %d\n",freeFramesAfter , initialFreeFrames);
	}
	if (is_correct) eval += 20;


	//====================================================================//
	/*ALLOCATE: Reallocate all pages using max block size (consume entire space)*/
	cprintf_colored(TEXT_cyan, "\n5: Reallocate all pages using max block size (consume entire space)  [20%]\n") ;
	curSize = DYN_ALLOC_MAX_BLOCK_SIZE ;
	numOfBlksAtCurSize = sizeDA / DYN_ALLOC_MAX_BLOCK_SIZE;
	is_correct = 1;
	for (int i = 1; i <= numOfBlksAtCurSize; ++i)
	{
		va = alloc_block(curSize);
		startVAsInit[i] = va; *startVAsInit[i] = i ;
		endVAsInit[i] = va + curSize - sizeof(short); *endVAsInit[i] = i ;
		if (check_dynalloc_datastruct(curSize, i) == 0)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "free_block test#8.%d: WRONG! DA data structures are not correct\n", i);
			break;
		}
	}
	//check content
	for (int i = 1; i <= numOfBlksAtCurSize; ++i)
	{
		if (*startVAsInit[i] != i || *endVAsInit[i] != i)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR, "free_block test#9.%d: WRONG! block content is changed while it's not expected to.", i);
			break;
		}
	}
	if (is_correct) eval += 20;

	cprintf_colored(TEXT_light_green, "test free_block completed. Evaluation = %d%\n", eval);

}

void test_realloc_block()
{
	panic("unseen test");

#if USE_KHEAP
	panic("test_realloc_block_COMPLETE: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return;
#endif
}


/********************Helper Functions***************************/
