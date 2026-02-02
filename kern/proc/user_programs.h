/*
 * user_programs.h
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#ifndef KERN_USER_PROGRAMS_H_
#define KERN_USER_PROGRAMS_H_
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

extern int NUM_USER_PROGS;

#define DECLARE_START_OF(binary_name)  \
	extern uint8 _binary_obj_user_##binary_name##_start[];

#define PTR_START_OF(binary_name) ( \
	(uint8*) _binary_obj_user_##binary_name##_start \
	)

//=========================================================
struct UserProgramInfo {
	const char *name;
	const char *desc;
	uint8* ptr_start;
};

struct UserProgramInfo*  get_user_program_info(char* user_program_name);
struct UserProgramInfo* get_user_program_info_by_env(struct Env* e);

///===================================================================================
/// To add FOS support for new user program, just add the appropriate lines like below

//The input for any DECLARE_START_OF macro must be the ".c" filename of the user program
/******************/
/*COMMON PROGRAMS */
/******************/
DECLARE_START_OF(fos_helloWorld)
DECLARE_START_OF(fos_add)
DECLARE_START_OF(fos_alloc)
DECLARE_START_OF(fos_input)
DECLARE_START_OF(game)
DECLARE_START_OF(fos_static_data_section)
DECLARE_START_OF(fos_data_on_stack)
DECLARE_START_OF(fos_fibonacci)
DECLARE_START_OF(fos_factorial)
DECLARE_START_OF(fib_memomize);
DECLARE_START_OF(fib_loop);
DECLARE_START_OF(mergesort_static);
DECLARE_START_OF(mergesort_leakage);
DECLARE_START_OF(mergesort_noleakage);
DECLARE_START_OF(quicksort_leakage)
DECLARE_START_OF(quicksort_noleakage);
DECLARE_START_OF(arrayOperations_Master);
DECLARE_START_OF(arrayOperations_quicksort);
DECLARE_START_OF(arrayOperations_mergesort);
DECLARE_START_OF(arrayOperations_stats);
DECLARE_START_OF(MidTermEx_Master);
DECLARE_START_OF(MidTermEx_ProcessA);
DECLARE_START_OF(MidTermEx_ProcessB);
DECLARE_START_OF(matrix_operations);
/********************************************/

/****************/
/*FAULT HANDLER */
/****************/
DECLARE_START_OF(tst_placement)
DECLARE_START_OF(tst_invalid_access)
DECLARE_START_OF(tst_invalid_access_slave1)
DECLARE_START_OF(tst_invalid_access_slave2)
DECLARE_START_OF(tst_invalid_access_slave3)
DECLARE_START_OF(tst_invalid_access_slave4)
/********************************************/
DECLARE_START_OF(tst_page_replacement_alloc)
DECLARE_START_OF(tst_page_replacement_stack)
DECLARE_START_OF(tst_page_replacement_optimal_1);
DECLARE_START_OF(tst_page_replacement_optimal_2);
DECLARE_START_OF(tst_page_replacement_optimal_3);
DECLARE_START_OF(tst_page_replacement_mod_clock_1);
DECLARE_START_OF(tst_page_replacement_mod_clock_2);
DECLARE_START_OF(tst_page_replacement_clock_1);
DECLARE_START_OF(tst_page_replacement_clock_2);
DECLARE_START_OF(tst_page_replacement_lru)
DECLARE_START_OF(dummy_process);
/********************************************/

/************/
/*USER HEAP */
/************/
DECLARE_START_OF(tst_malloc_1);
DECLARE_START_OF(tst_malloc_2);
/********************************************/
DECLARE_START_OF(tst_free_1);
/*2023*/DECLARE_START_OF(tst_free_1_slave1);
/*2023*/DECLARE_START_OF(tst_free_1_slave2);
DECLARE_START_OF(tst_free_2);
/********************************************/
DECLARE_START_OF(tst_custom_fit_1);
DECLARE_START_OF(tst_custom_fit_2);
DECLARE_START_OF(tst_custom_fit_3);
/********************************************/
DECLARE_START_OF(heap_program);
DECLARE_START_OF(tst_quicksort_freeHeap)
/********************************************/
DECLARE_START_OF(tst_da_block_master);
DECLARE_START_OF(tst_da_block_slave);
/********************************************/


/****************/
/*SHARED MEMORY */
/****************/
DECLARE_START_OF(tst_sharing_1);
DECLARE_START_OF(tst_sharing_2master);
DECLARE_START_OF(tst_sharing_2slave1);
DECLARE_START_OF(tst_sharing_2slave2);
DECLARE_START_OF(tst_sharing_3);
/********************************************/

/*****************/
/*CPU SCHEDULING */
/*****************/
DECLARE_START_OF(priRR_fib_create);
DECLARE_START_OF(priRR_fib_small);
DECLARE_START_OF(priRR_fib);
DECLARE_START_OF(priRR_fib_pri4);
DECLARE_START_OF(priRR_fib_pri8);
/********************************************/

/**************/
/*CONCURRENCY */
/**************/
DECLARE_START_OF(concurrent_start)
DECLARE_START_OF(tst_air);
DECLARE_START_OF(tst_air_clerk);
DECLARE_START_OF(tst_air_customer);
/********************************************/
DECLARE_START_OF(tst_ksemaphore_1master);
DECLARE_START_OF(tst_ksemaphore_1slave);
DECLARE_START_OF(tst_ksemaphore_2master);
DECLARE_START_OF(tst_ksemaphore_2slave);
/********************************************/
DECLARE_START_OF(tst_chan_all_master);
DECLARE_START_OF(tst_chan_all_slave);
DECLARE_START_OF(tst_chan_one_master);
DECLARE_START_OF(tst_chan_one_slave);
/********************************************/
DECLARE_START_OF(tst_sleeplock_master);
DECLARE_START_OF(tst_sleeplock_slave);
/********************************************/
DECLARE_START_OF(tst_protection);
DECLARE_START_OF(tst_protection_slave1);
/********************************************/

/******************/
/*EXIT (env_free) */
/******************/
DECLARE_START_OF(tst_envfree1);
DECLARE_START_OF(tst_envfree2);
DECLARE_START_OF(tst_envfree3);
DECLARE_START_OF(tst_envfree3_slave);
DECLARE_START_OF(tst_envfree4);
DECLARE_START_OF(tst_envfree5_1);
DECLARE_START_OF(tst_envfree5_2);
DECLARE_START_OF(tst_envfree6);
/********************************************/
DECLARE_START_OF(ef_mergesort_noleakage);
DECLARE_START_OF(ef_mergesort_leakage);
DECLARE_START_OF(ef_tst_sharing_1);
DECLARE_START_OF(ef_tst_sharing_2master);
DECLARE_START_OF(ef_tst_sharing_2slave1);
DECLARE_START_OF(ef_tst_semaphore_1master);
DECLARE_START_OF(ef_tst_semaphore_1slave);
DECLARE_START_OF(ef_tst_sharing_4);
DECLARE_START_OF(ef_tst_sharing_5_master);
DECLARE_START_OF(ef_tst_sharing_5_slave);
DECLARE_START_OF(ef_tst_sharing_5_slaveB1);
DECLARE_START_OF(ef_tst_sharing_5_slaveB2);
DECLARE_START_OF(ef_MidTermEx_Master);
/********************************************/


#endif /* KERN_USER_PROGRAMS_H_ */
