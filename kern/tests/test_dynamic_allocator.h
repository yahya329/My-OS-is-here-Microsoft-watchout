/*
 * test_dynamic_allocator.h
 *
 *  Created on: Oct 14, 2022
 *      Author: HP
 */

#ifndef KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_
#define KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

/*2025*/
int  test_initialize_dynamic_allocator();
void test_alloc_block();
void test_free_block();
void test_realloc_block();
int check_dynalloc_datastruct(void* va, void* expectedVA, uint32 expectedSize, uint8 expectedFlag);


#endif /* KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_ */
