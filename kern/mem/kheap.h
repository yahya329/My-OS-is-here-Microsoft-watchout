#ifndef FOS_KERN_KHEAP_H_
#define FOS_KERN_KHEAP_H_
#include <inc/queue.h>
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

#include <inc/queue.h>
#include <inc/types.h>


/*2017*/
//Values for user heap placement strategy
#define KHP_PLACE_CONTALLOC 0x0
#define KHP_PLACE_FIRSTFIT 	0x1
#define KHP_PLACE_BESTFIT 	0x2
#define KHP_PLACE_NEXTFIT 	0x3
#define KHP_PLACE_WORSTFIT 	0x4
#define KHP_PLACE_CUSTOMFIT 0x5

//[3] Data Structure
struct Region {
	uint32 VA;
	uint32 size;
	bool isFree; //free = 1 , allocated = 0
	LIST_ENTRY(Region)  prev_next_info;
};

LIST_HEAD(RegionList,Region);

struct RegionList KRegionList;


//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #0 Page Alloc Limits [GIVEN]
uint32 kheapPageAllocStart ;
uint32 kheapPageAllocBreak ;
uint32 kheapPlacementStrategy;

/*2025*/ //Replaced by setter & getter function
static inline void set_kheap_strategy(uint32 strategy){kheapPlacementStrategy = strategy;}
static inline uint32 get_kheap_strategy(){return kheapPlacementStrategy ;}

//***********************************
void kheap_init();

void* kmalloc(unsigned int size);
void kfree(void* virtual_address);
void *krealloc(void *virtual_address, unsigned int new_size);

unsigned int kheap_virtual_address(unsigned int physical_address);
unsigned int kheap_physical_address(unsigned int virtual_address);

int numOfKheapVACalls ;


#endif // FOS_KERN_KHEAP_H_
