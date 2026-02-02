/*
 * test_kheap.h
 *
 *  Created on: Oct 14, 2022
 *      Author: HP
 */

#ifndef KERN_TESTS_TEST_KHEAP_H_
#define KERN_TESTS_TEST_KHEAP_H_
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif
#include <inc/types.h>
#define TST_PAGE_ALLOC 		0x1
#define TST_BLOCK_ALLOC 	0x2
#define TST_BOTH_ALLOC 		0x3

//2016: Kernel Heap Tests
 int test_kmalloc(uint32 ALLOC_TYPE);
 int test_kfree(uint32 ALLOC_TYPE);
 int test_krealloc(uint32 ALLOC_TYPE);
 int test_kheap_phys_addr();
 int test_kheap_virt_addr();
 int test_fast_page_alloc();
 int test_three_creation_functions();
 int test_ksbrk();

 int test_kfreeall();
 int test_kexpand();
 int test_kshrink();
 int test_kfreelast();

 //2022
 int test_initialize_dyn_block_system(int freeFrames_before, int freeDiskFrames_before, int freeFrames_after, int freeDiskFrames_after);

#endif /* KERN_TESTS_TEST_KHEAP_H_ */
