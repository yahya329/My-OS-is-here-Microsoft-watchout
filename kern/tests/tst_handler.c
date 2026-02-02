/*
 * tst_handler.c
 *
 *  Created on: Oct 9, 2023
 *      Author: HP
 */
#include "../tests/tst_handler.h"
#include <kern/trap/trap.h>
#include <kern/trap/fault_handler.h>
#include <kern/proc/user_environment.h>
#include <kern/proc/priority_manager.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/kheap.h"
#include "../mem/memory_manager.h"
#include "../tests/utilities.h"
#include "../tests/test_kheap.h"
#include "../tests/test_priority.h"
#include "../tests/test_commands.h"
#include "../tests/test_dynamic_allocator.h"
#include "../tests/test_scheduler.h"

struct Test tests[] = {
		{"3functions", "Env Load: test the creation of new dir, tables and pages WS", tst_three_creation_functions},
		{"kfreeall", "Kernel Heap: test kfreeall (freed frames, mem access...etc)", tst_kfreeall},
		{"kexpand", "Kernel Heap: test expanding last allocated var", tst_kexpand},
		{"kshrink", "Kernel Heap: test shrinking last allocated var", tst_kshrink},
		{"kfreelast", "Kernel Heap: test freeing last allocated var", tst_kfreelast},
		{"priority1", "Tests the priority of the program (Normal and Higher)", tst_priority1},
		{"priority2", "Tests the priority of the program (Normal and Lower)", tst_priority2},
		{"mlfq_sc4","Scenario#4: MLFQ",tst_sc_MLFQ },
		{"bsd_nice", "BSD Scheduler: check order of running multiple instances of same program with different nice values", tst_bsd_nice},
		{"priorityRR", "Priority RR Scheduler: check order of running multiple instances of same program with different priority values", tst_priorityRR},

		//2022
		{"str2lower", "Test str2lower function", tst_str2lower},
		{"autocomplete", "Test commands autcomplte", tst_autocomplete},
		{"dynalloc","Test dynamic allocator", tst_dyn_alloc },
		{"pg", "Test paging manipulation for a specific page", tst_paging_manipulation},
		{"chunks","Test chunk manipulations", tst_chunks },
		{"kheap", "Test KHEAP functions", tst_kheap},

};

//Number of tests = size of the array / size of test structure
uint32 NUM_OF_TESTS = (sizeof(tests)/sizeof(struct Test));


//=================//
/*Test MAIN Handler*/
//=================//
int tst_handler(int number_of_arguments, char **arguments)
{
	//Remove "tst" from arguments
	for (int a = 0; a < number_of_arguments - 1; ++a)
	{
		arguments[a] = arguments[a+1] ;
	}
	number_of_arguments--;

	//Check name of the given test and execute its corresponding function
	int test_found = 0;
	int i ;
	for (i = 0; i < NUM_OF_TESTS; i++)
	{
		if (strcmp(arguments[0], tests[i].name) == 0)
		{
			test_found = 1;
			break;
		}
	}

	if(test_found)
	{
		int return_value;
		return_value = tests[i].function_to_execute(number_of_arguments, arguments);
		return return_value;
	}
	else
	{
		cprintf("Unknown test '%s'\n", arguments[0]);
		return 0;
	}
}

//=================//
/*TESTING Functions*/
//=================//
int tst_three_creation_functions(int number_of_arguments, char **arguments)
{
	test_three_creation_functions();
	return 0;
}

int tst_priority1(int number_of_arguments, char **arguments)
{
	test_priority_normal_and_higher();
	return 0;
}

int tst_priority2(int number_of_arguments, char **arguments)
{
	test_priority_normal_and_lower();
	return 0;
}

int tst_kfreeall(int number_of_arguments, char **arguments)
{
	test_kfreeall();
	return 0;
}

int tst_kexpand(int number_of_arguments, char **arguments)
{
	test_kexpand();
	return 0;
}

int tst_kshrink(int number_of_arguments, char **arguments)
{
	test_kshrink();
	return 0;
}

int tst_kfreelast(int number_of_arguments, char **arguments)
{
	test_kfreelast();
	return 0;
}

int tst_sc_MLFQ(int number_of_arguments, char **arguments)
{
	int numOfSlave2 = strtol(arguments[1], NULL, 10);
	int cnt = 0 ;
	int firstTime = 1;
	struct Env *e ;
	acquire_kspinlock(&ProcessQueues.qlock);
	{
		LIST_FOREACH(e, &ProcessQueues.env_exit_queue)
									{
			if (strcmp(e->prog_name, "tmlfq_2") == 0)
			{
				if (firstTime)
					firstTime = 0;
				cnt++ ;
			}
			else if (!firstTime)
				break;
									}
		if(cnt == numOfSlave2)
		{
			cprintf("Congratulations... MLFQScenario# completed successfully\n");
		}
		else
		{
			panic("MLFQScenario# failed\n");
		}
	}
	release_kspinlock(&ProcessQueues.qlock);
	return 0;
}


/*2023*/
int tst_bsd_nice(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 2)
	{
		cprintf("Invalid number of arguments! USAGE: tst bsd_nice <testnumber>\n");
		return 0;
	}
	int testNumber = strtol(arguments[1], NULL, 10);
	switch (testNumber)
	{
	case 0:
		test_bsd_nice_0();
		break;
	case 1:
		test_bsd_nice_1();
		break;
	case 2:
		test_bsd_nice_2();
		break;
	}
	return 0;
}

/*2024*/
int tst_priorityRR(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 2)
	{
		cprintf("Invalid number of arguments! USAGE: tst priorityRR <testnumber>\n");
		return 0;
	}
	int testNumber = strtol(arguments[1], NULL, 10);
	switch (testNumber)
	{
	case 0:
		test_priorityRR_0();
		break;
	case 1:
		test_priorityRR_1();
		break;
	case 2:
		test_priorityRR_2();
		break;
	}
	return 0;
}
int tst_str2lower(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 1)
	{
		cprintf("Invalid number of arguments! USAGE: tst str2lower\n");
		return 0;
	}

	test_str2lower_function();
	return 0;
}

int tst_autocomplete(int number_of_arguments, char **arguments)
{
	int x = TestAutoCompleteCommand();
	return 0;
}
int tst_dyn_alloc(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 2)
	{
		cprintf("Invalid number of arguments! USAGE: tst dynalloc <testname>\n") ;
		return 0;
	}
	//str2lower(arguments[1]);
	// Test 1 Example for initialize_MemBlocksList: tstdynalloc init
	if(strcmp(arguments[1], "init") == 0)
	{
		test_initialize_dynamic_allocator();
	}
	// Test 2 Example for alloc_block: tst dynalloc alloc
	else if(strcmp(arguments[1], "alloc") == 0)
	{
		test_alloc_block();
	}
	/*
	// Test 2 Example for alloc_block_FF: tstdynalloc allocFF
	else if(strcmp(arguments[1], "allocff") == 0)
	{
		test_alloc_block_FF();
	}
	// Test 3 Example for alloc_block_BF: tstdynalloc allocBF
	else if(strcmp(arguments[1], "allocbf") == 0)
	{
		test_alloc_block_BF();
	}
	// Test 4 Example for alloc_block_NF: tstdynalloc allocNF
	else if(strcmp(arguments[1], "allocnf") == 0)
	{
		test_alloc_block_NF();
	}
	 */
	// Test 5 Example for free_block: tst dynalloc free
	else if(strcmp(arguments[1], "free") == 0)
	{
		test_free_block();
	}
	/*	// Test 5 Example for free_block: tstdynalloc freeFF
	else if(strcmp(arguments[1], "freeff") == 0)
	{
		test_free_block_FF();
	}
	// Test 6 Example for free_block: tstdynalloc freeBF
	else if(strcmp(arguments[1], "freebf") == 0)
	{
		test_free_block_BF();
	}
	// Test 7 Example for free_block: tstdynalloc freeNF
	else if(strcmp(arguments[1], "freenf") == 0)
	{
		test_free_block_NF();
	}*/
	// Test 8 Example for realloc_block: tst dynalloc realloc
	else if(strcmp(arguments[1], "realloc") == 0)
	{
		test_realloc_block();
	}
	return 0;
}

int tst_chunks(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 2)
	{
		cprintf("Invalid number of arguments! USAGE: tst chunks <testname>\n") ;
		return 0;
	}
	// CUT-PASTE Test
	if(strcmp(arguments[1], "cutpaste") == 0)
	{
		test_cut_paste_pages();
	}
	// COPY-PASTE Test
	else if(strcmp(arguments[1], "copypaste") == 0)
	{
		test_copy_paste_chunk();
	}
	// SHARE Test
	else if(strcmp(arguments[1], "share") == 0)
	{
		test_share_chunk();
	}
	// ALLOCATE Test
	else if(strcmp(arguments[1], "allocate") == 0)
	{
		test_allocate_chunk();
	}
	// REQUIRED SPACE Test
	else if(strcmp(arguments[1], "required_space") == 0)
	{
		test_calculate_required_frames();
	}
	// ALLOCATED SPACE Test
	else if(strcmp(arguments[1], "allocated_space") == 0)
	{
		test_calculate_allocated_space();
	}
	return 0;
}

int tst_paging_manipulation(int number_of_arguments, char **arguments)
{
	if (number_of_arguments != 2)
	{
		cprintf("Invalid number of arguments! USAGE: tst pg <testname>\n") ;
		return 0;
	}
	// Test 1.1-Set/Clear permissions: tst pg scperm1
	if(strcmp(arguments[1], "scperm1") == 0)
	{
		test_pt_set_page_permissions();
	}
	// Test 1.2-Set/Clear permissions: tst pg scperm2
	else if(strcmp(arguments[1], "scperm2") == 0)
	{
		test_pt_set_page_permissions_invalid_va();
	}
	// Test 2-Get permissions: tst pg getperm
	else if(strcmp(arguments[1], "getperm") == 0)
	{
		test_pt_get_page_permissions();
	}
	// Test 3.1-Clear entry: tst pg clear1
	else if(strcmp(arguments[1], "clear1") == 0)
	{
		test_pt_clear_page_table_entry();
	}
	// Test 3.2-Clear entry: tst pg clear2
	else if(strcmp(arguments[1], "clear2") == 0)
	{
		test_pt_clear_page_table_entry_invalid_va();
	}
	// Test 4-Convert virtual to physical: tst pg v2p
	else if(strcmp(arguments[1], "v2p") == 0)
	{
		test_virtual_to_physical();
	}
	return 0;
}

int tst_kheap(int number_of_arguments, char **arguments)
{
#if !USE_KHEAP
	panic("MUST ENABLE KHEAP");
	return 0;
#endif
	// Parameters Validation Checking
	if (strcmp(arguments[2], "kmalloc") == 0 && number_of_arguments != 4)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> kmalloc <both or blk or page>\n") ;
		return 0;
	}
	else if (strcmp(arguments[2], "fast") == 0 && number_of_arguments != 3)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> fast\n") ;
		return 0;
	}
	else if (strcmp(arguments[2], "kfree") == 0 && number_of_arguments != 4)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> kfree <both or blk or page>\n") ;
		return 0;
	}
	else if (strcmp(arguments[2], "kvirtaddr") == 0 && number_of_arguments != 3)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> kvirtaddr\n") ;
		return 0;
	}
	else if (strcmp(arguments[2], "kphysaddr") == 0 && number_of_arguments != 3)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> kphysaddr\n") ;
		return 0;
	}
	else if (strcmp(arguments[2], "krealloc") == 0 && number_of_arguments != 4)
	{
		cprintf("Invalid number of arguments! USAGE: tst kheap <Strategy> krealloc <both or blk or page>\n") ;
		return 0;
	}

	// Specify Test Type [if any]
	uint32 testType = 0;
	if (number_of_arguments == 4)
	{
		if (strcmp(arguments[3], "page") == 0)
		{
			testType = TST_PAGE_ALLOC;
		}
		else if (strcmp(arguments[3], "blk") == 0)
		{
			testType = TST_BLOCK_ALLOC;
		}
		else if (strcmp(arguments[3], "both") == 0)
		{
			testType = TST_BOTH_ALLOC;
		}
		else
		{
			cprintf("Invalid Allocator Type! <both or blk or page>\n") ;
			return 0;
		}
	}
	// Setting Strategy
	if(strcmp(arguments[1], "FF") == 0 || strcmp(arguments[1], "ff") == 0)
	{
		set_kheap_strategy(KHP_PLACE_FIRSTFIT);
		cprintf("Kernel Heap placement strategy is FIRST FIT\n");
	}
	else if(strcmp(arguments[1], "BF") == 0 || strcmp(arguments[1], "bf") == 0)
	{
		set_kheap_strategy(KHP_PLACE_BESTFIT);
		cprintf("Kernel Heap placement strategy is BEST FIT\n");
	}
	else if(strcmp(arguments[1], "NF") == 0 || strcmp(arguments[1], "nf") == 0)
	{
		set_kheap_strategy(KHP_PLACE_NEXTFIT);
		cprintf("Kernel Heap placement strategy is NEXT FIT\n");
	}
	else if(strcmp(arguments[1], "WF") == 0 || strcmp(arguments[1], "wf") == 0)
	{
		set_kheap_strategy(KHP_PLACE_WORSTFIT);
		cprintf("Kernel Heap placement strategy is WORST FIT\n");
	}
	else if(strcmp(arguments[1], "CF") == 0 || strcmp(arguments[1], "cf") == 0)
	{
		set_kheap_strategy(KHP_PLACE_CUSTOMFIT);
		cprintf("Kernel Heap placement strategy is CUSTOM FIT\n");
	}

	// Test 1-kmalloc: tst kheap <Strategy> kmalloc <allocator>
	if(strcmp(arguments[2], "kmalloc") == 0)
	{
		test_kmalloc(testType);
		return 0;
	}
	// Test Fast Implementation of kmalloc/kfree: tst kheap <Startegy> fast
	else if(strcmp(arguments[2], "fast") == 0)
	{
		test_fast_page_alloc();
		return 0;
	}
	// Test 2-kfree: tst kheap <Strategy> kfree <allocator>
	else if(strcmp(arguments[2], "kfree") == 0)
	{
		test_kfree(testType);
		return 0;
	}
	// Test 3-kphysaddr: tst kheap <Strategy> kphysaddr
	// <Strategy> IS NEGLECTED
	else if(strcmp(arguments[2], "kphysaddr") == 0)
	{
		test_kheap_phys_addr();
		return 0;
	}
	// Test 4-kvirtaddr: tst kheap <Strategy> kvirtaddr
	// <Strategy> IS NEGLECTED
	else if(strcmp(arguments[2], "kvirtaddr") == 0)
	{
		test_kheap_virt_addr();
		return 0;
	}
	// Test 5-krealloc: tst kheap <Strategy> krealloc <allocator>
	else if(strcmp(arguments[2], "krealloc") == 0)
	{
		test_krealloc(testType);
		return 0;
	}
	/*	// Test 6-sbr: tst kheap FF sbrk
	else if (strcmp(arguments[2], "sbrk") == 0)
	{
		test_ksbrk();
	}*/
	return 0;
}


//END======================================================

