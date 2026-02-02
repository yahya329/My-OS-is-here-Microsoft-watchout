/*
 * user_programs.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */
#include <kern/proc/user_environment.h>
#include <inc/string.h>
#include <inc/assert.h>

//User Programs Table
//The input for any PTR_START_OF macro must be the ".c" filename of the user program
struct UserProgramInfo userPrograms[] = {

		/******************/
		/*COMMON PROGRAMS */
		/******************/
		{ "fos_helloWorld", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_helloWorld)},
		{ "fos_add", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_add)},
		{ "fos_alloc", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_alloc)},
		{ "fos_input", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_input)},
		{ "fos_game", "Created by FOS team, fos@nowhere.com", PTR_START_OF(game)},
		{ "fos_static_data_section", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_static_data_section)},
		{ "fos_data_on_stack", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_data_on_stack)},
		{ "fib_memomize", "", PTR_START_OF(fib_memomize)},
		{ "fib_loop", "", PTR_START_OF(fib_loop)},
		{ "fact", "Factorial Recursive", PTR_START_OF(fos_factorial)},
		{ "fib", "Fibonacci Recursive", PTR_START_OF(fos_fibonacci)},
		{ "qs1", "Quicksort with NO memory leakage", PTR_START_OF(quicksort_noleakage)},
		{ "qs2", "Quicksort that cause memory leakage", PTR_START_OF(quicksort_leakage)},
		{ "mergesort", "mergesort a fixed size array of 800000", PTR_START_OF(mergesort_static)},
		{ "ms1", "Mergesort with NO memory leakage", PTR_START_OF(mergesort_noleakage)},
		{ "ms2", "Mergesort that cause memory leakage", PTR_START_OF(mergesort_leakage)},
		{ "arrop", "Apply set of array operations: scenario program to test semaphores & shared objects", PTR_START_OF(arrayOperations_Master)},
		{ "slave_qs", "SlaveOperation: quicksort", PTR_START_OF(arrayOperations_quicksort)},
		{ "slave_ms", "SlaveOperation: mergesort", PTR_START_OF(arrayOperations_mergesort)},
		{ "slave_stats", "SlaveOperation: stats", PTR_START_OF(arrayOperations_stats)},
		{ "midterm", "Midterm 2017: Example on shared resource and dependency", PTR_START_OF(MidTermEx_Master)},
		{ "midterm_a", "Midterm 2017 Example: Process A", PTR_START_OF(MidTermEx_ProcessA)},
		{ "midterm_b", "Midterm 2017 Example: Process B", PTR_START_OF(MidTermEx_ProcessB)},
		{ "matops", "Matrix Operations on two square matrices with NO memory leakage", PTR_START_OF(matrix_operations)},
		/****************/
		/*FAULT HANDLER */
		/****************/
		{ "tpp", "Tests the Page placement", PTR_START_OF(tst_placement)},
		{ "tia", "tests handling of invalid memory access", PTR_START_OF(tst_invalid_access)},
		{ "tia_slave1", "tia: access kernel", PTR_START_OF(tst_invalid_access_slave1)},
		{ "tia_slave2", "tia: write on read only user page", PTR_START_OF(tst_invalid_access_slave2)},
		{ "tia_slave3", "tia: access an unmarked (non-reserved) user heap page", PTR_START_OF(tst_invalid_access_slave3)},
		{ "tia_slave4", "tia: access a non-exist page in page file, stack and heap", PTR_START_OF(tst_invalid_access_slave4)},
		/********************************************/
		{ "tpr1", "Tests page replacement (allocation of Memory and PageFile)", PTR_START_OF(tst_page_replacement_alloc)},
		{ "tpr2", "tests page replacement (handling new stack and modified pages)", PTR_START_OF(tst_page_replacement_stack)},
		{ "dummy_process", "[Slave program] contains nested loops with random bounds to consume time", PTR_START_OF(dummy_process)},
		{ "toptimal1", "Tests page replacement (OPTIMAL algorithm 1) - no change in DS", PTR_START_OF(tst_page_replacement_optimal_1)},
		{ "toptimal2", "Tests page replacement (OPTIMAL algorithm 2) - placement", PTR_START_OF(tst_page_replacement_optimal_2)},
		{ "toptimal3", "Tests page replacement (OPTIMAL algorithm 3) - replacement", PTR_START_OF(tst_page_replacement_optimal_3)},
		{ "tclock1", "Tests page replacement (clock algorithm)", PTR_START_OF(tst_page_replacement_clock_1)},
		{ "tclock2", "Tests page replacement (clock algorithm) after free", PTR_START_OF(tst_page_replacement_clock_2)},
		{ "tmodclk1", "Tests page replacement (modified clock algorithm)", PTR_START_OF(tst_page_replacement_mod_clock_1)},
		{ "tmodclk2", "Tests page replacement (modified clock algorithm) after free", PTR_START_OF(tst_page_replacement_mod_clock_2)},
		{ "tlru", "Tests page replacement (LRU algorithm)", PTR_START_OF(tst_page_replacement_lru)},
		/********************************************/
		/************/
		/*USER HEAP */
		/************/
		{ "tm1", "tests malloc (1): start address & allocated frames", PTR_START_OF(tst_malloc_1)},
		{ "tm2", "tests malloc (2): writing & reading values in allocated spaces", PTR_START_OF(tst_malloc_2)},
		/********************************************/
		{ "tf1", "tests free (1): freeing tables, WS and page file [placement case]", PTR_START_OF(tst_free_1)},
		{ "tf1_slave1", "tests free (1) slave1: try accessing values in freed spaces", PTR_START_OF(tst_free_1_slave1)},
		{ "tf1_slave2", "tests free (1) slave2: try accessing values in freed spaces that is not accessed before", PTR_START_OF(tst_free_1_slave2)},
		{ "tf2", "tests free (2): try accessing values in freed spaces", PTR_START_OF(tst_free_2)},
		/********************************************/
		{ "tcf1", "tests custom fit (1): page allocator", PTR_START_OF(tst_custom_fit_1)},
		{ "tcf2", "tests custom fit (2): block allocator", PTR_START_OF(tst_custom_fit_2)},
		{ "tcf3", "tests custom fit (3): malloc, smalloc & sget", PTR_START_OF(tst_custom_fit_3)},
		/********************************************/
		{ "hp", "heap program (allocate and free from heap)", PTR_START_OF(heap_program)},
		{ "tqsfh", "Quicksort with freeHeap", PTR_START_OF(tst_quicksort_freeHeap)},
		/********************************************/
		{ "tst_da_block", "DA: Test blocking the thread if no block is available", PTR_START_OF(tst_da_block_master)},
		{ "tst_da_block_slave", "Slave program of tst_da_block", PTR_START_OF(tst_da_block_slave)},
		/********************************************/
		/****************/
		/*SHARED MEMORY */
		/****************/
		{ "tshr1", "Tests the shared variables [create]", PTR_START_OF(tst_sharing_1)},
		{ "tshr2", "Tests the shared variables [create, get and perms]", PTR_START_OF(tst_sharing_2master)},
		{ "shr2Slave1", "[Slave program1] of tst_sharing_2master", PTR_START_OF(tst_sharing_2slave1)},
		{ "shr2Slave2", "[Slave program2] of tst_sharing_2master", PTR_START_OF(tst_sharing_2slave2)},
		{ "tshr3", "Tests the shared variables [Special cases of create]", PTR_START_OF(tst_sharing_3)},
		/********************************************/
		/*****************/
		/*CPU SCHEDULING */
		/*****************/
		{ "priRR_fib_create", "Create and run TWO instances of Fibonacci 38 at priority 2 & 9", PTR_START_OF(priRR_fib_create)},
		{ "priRR_fib", "Fibonacci 38", PTR_START_OF(priRR_fib)},
		{ "priRR_fib_small", "Fibonacci 8", PTR_START_OF(priRR_fib_small)},
		{ "priRR_fib_pri4", "Fibonacci 38 with priority 4", PTR_START_OF(priRR_fib_pri4)},
		{ "priRR_fib_pri8", "Fibonacci 38 with priority 8", PTR_START_OF(priRR_fib_pri8)},
		/********************************************/
		/**************/
		/*CONCURRENCY */
		/**************/
		{ "cnc", "Concurrent program test", PTR_START_OF(concurrent_start)},
		{ "tair", "", PTR_START_OF(tst_air)},
		{ "taircl", "", PTR_START_OF(tst_air_clerk)},
		{ "taircu", "", PTR_START_OF(tst_air_customer)},
		/********************************************/
		{ "tst_ksem1", "Tests the KERNEL Semaphores only [critical section & dependency]", PTR_START_OF(tst_ksemaphore_1master)},
		{ "ksem1Slave", "[Slave program] of tst_ksemaphore_1master", PTR_START_OF(tst_ksemaphore_1slave)},
		{ "tst_ksem2", "Tests the KERNEL Semaphores only [multiprograms enter the same CS]", PTR_START_OF(tst_ksemaphore_2master)},
		{ "ksem2Slave", "[Slave program] of tst_ksemaphore_2master", PTR_START_OF(tst_ksemaphore_2slave)},
		/********************************************/
		{ "tst_chan_all", "Tests sleep & wakeup ALL on a channel", PTR_START_OF(tst_chan_all_master)},
		{ "tstChanAllSlave", "Slave program of tst_chan_all", PTR_START_OF(tst_chan_all_slave)},
		{ "tst_chan_one", "Tests sleep & wakeup ONE on a channel", PTR_START_OF(tst_chan_one_master)},
		{ "tstChanOneSlave", "Slave program of tst_chan_one", PTR_START_OF(tst_chan_one_slave)},
		/********************************************/
		{ "tst_sleeplock", "Tests the acquire & release of sleep lock", PTR_START_OF(tst_sleeplock_master)},
		{ "tstSleepLockSlave", "Slave program of tst_sleeplock", PTR_START_OF(tst_sleeplock_slave)},
		/********************************************/
		{ "tst_protection", "Tests the protection of kernel shared DS (e.g. kernel heap)", PTR_START_OF(tst_protection)},
		{ "protection_slave1", "Slave program of tst_protection", PTR_START_OF(tst_protection_slave1)},
		/********************************************/
		/******************/
		/*EXIT (env_free) */
		/******************/
		{ "tef1", "test env_free#1: without user heap [PLACEMENT]", PTR_START_OF(tst_envfree1)},
		{ "tef2", "test env_free#2: using user heap [REPLACEMENT]", PTR_START_OF(tst_envfree2)},
		{ "tef3", "test env_free#3: using user heap & process kill iteself [REPLACEMENT]", PTR_START_OF(tst_envfree3)},
		{ "tef3_slave", "", PTR_START_OF(tst_envfree3_slave)},
		{ "tef4", "test env_free#4: using shared memory (create & get) [REPLACEMENT]", PTR_START_OF(tst_envfree4)},
		{ "tef5_1", "", PTR_START_OF(tst_envfree5_1)},
		{ "tef5_2", "test env_free#5: using shared memory (create, get & free) [PLACEMENT]", PTR_START_OF(tst_envfree5_2)},
		{ "tef6", "test env_free#6: using shared memory (create & get) + semaphores [PLACEMENT]", PTR_START_OF(tst_envfree6)},
		/********************************************/
		{ "ef_ms1", "", PTR_START_OF(ef_mergesort_noleakage)},
		{ "ef_ms2", "", PTR_START_OF(ef_mergesort_leakage)},
		{ "ef_tshr1", "", PTR_START_OF(ef_tst_sharing_1)},
		{ "ef_tshr2", "", PTR_START_OF(ef_tst_sharing_2master)},
		{ "ef_shr2Slave1", "", PTR_START_OF(ef_tst_sharing_2slave1)},
		{ "ef_tsem1", "", PTR_START_OF(ef_tst_semaphore_1master)},
		{ "ef_sem1Slave", "", PTR_START_OF(ef_tst_semaphore_1slave)},
		{ "ef_tshr4", "", PTR_START_OF(ef_tst_sharing_4)},
		{ "ef_tshr5", "", PTR_START_OF(ef_tst_sharing_5_master)},
		{ "ef_tshr5slave", "", PTR_START_OF(ef_tst_sharing_5_slave)},
		{ "ef_tshr5slaveB1", "", PTR_START_OF(ef_tst_sharing_5_slaveB1)},
		{ "ef_tshr5slaveB2", "", PTR_START_OF(ef_tst_sharing_5_slaveB2)},
		{ "ef_midterm", "", PTR_START_OF(ef_MidTermEx_Master)},
		/********************************************/
};

///=========================================================

// To be used as extern in other files
struct UserProgramInfo* ptr_UserPrograms = &userPrograms[0];

// Number of user programs in the program table
int NUM_USER_PROGS = (sizeof(userPrograms)/sizeof(userPrograms[0]));

struct UserProgramInfo* get_user_program_info(char* user_program_name)
{
	int i;
	for (i = 0; i < NUM_USER_PROGS; i++) {
		if (strcmp(user_program_name, userPrograms[i].name) == 0)
			break;
	}
	if(i==NUM_USER_PROGS)
	{
		cprintf("Unknown user program '%s'\n", user_program_name);
		return 0;
	}

	return &userPrograms[i];
}

struct UserProgramInfo* get_user_program_info_by_env(struct Env* e)
{
	int i;
	for (i = 0; i < NUM_USER_PROGS; i++) {
		if ( strcmp( e->prog_name , userPrograms[i].name) ==0)
			break;
	}
	if(i==NUM_USER_PROGS)
	{
		cprintf("Unknown user program \n");
		return 0;
	}

	return &userPrograms[i];
}
