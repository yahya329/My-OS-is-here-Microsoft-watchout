/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/kheap.h>

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}
/*2024*/ void setFASTNchanceCLOCK(bool fast){ FASTNchanceCLOCK = fast; };
/*2025*/ void setPageReplacmentAlgorithmOPTIMAL(){ _PageRepAlgoType = PG_REP_OPTIMAL; };

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmOPTIMAL(){if(_PageRepAlgoType == PG_REP_OPTIMAL) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//==================
// [0] INIT HANDLER:
//==================
void fault_handler_init()
{
	//setPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX);
	//setPageReplacmentAlgorithmOPTIMAL();
	setPageReplacmentAlgorithmCLOCK();
	//setPageReplacmentAlgorithmModifiedCLOCK();
	enableBuffering(0);
	enableModifiedBuffer(0) ;
	setModifiedBufferLength(1000);
}
//==================
// [1] MAIN HANDLER:
//==================
/*2022*/
uint32 last_eip = 0;
uint32 before_last_eip = 0;
uint32 last_fault_va = 0;
uint32 before_last_fault_va = 0;
int8 num_repeated_fault  = 0;
extern uint32 sys_calculate_free_frames() ;

struct Env* last_faulted_env = NULL;
void fault_handler(struct Trapframe *tf)
{
	/******************************************************/
	// Read processor's CR2 register to find the faulting address
	uint32 fault_va = rcr2();
	//cprintf("************Faulted VA = %x************\n", fault_va);
	//	print_trapframe(tf);
	/******************************************************/

	//If same fault va for 3 times, then panic
	//UPDATE: 3 FAULTS MUST come from the same environment (or the kernel)
	struct Env* cur_env = get_cpu_proc();
	if (last_fault_va == fault_va && last_faulted_env == cur_env)
	{
		num_repeated_fault++ ;
		if (num_repeated_fault == 3)
		{
			print_trapframe(tf);
			panic("Failed to handle fault! fault @ at va = %x from eip = %x causes va (%x) to be faulted for 3 successive times\n", before_last_fault_va, before_last_eip, fault_va);
		}
	}
	else
	{
		before_last_fault_va = last_fault_va;
		before_last_eip = last_eip;
		num_repeated_fault = 0;
	}
	last_eip = (uint32)tf->tf_eip;
	last_fault_va = fault_va ;
	last_faulted_env = cur_env;
	/******************************************************/
	//2017: Check stack overflow for Kernel
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	if (!userTrap)
	{
		struct cpu* c = mycpu();
		//cprintf("trap from KERNEL\n");
		if (cur_env && fault_va >= (uint32)cur_env->kstack && fault_va < (uint32)cur_env->kstack + PAGE_SIZE)
			panic("User Kernel Stack: overflow exception!");
		else if (fault_va >= (uint32)c->stack && fault_va < (uint32)c->stack + PAGE_SIZE)
			panic("Sched Kernel Stack of CPU #%d: overflow exception!", c - CPUS);
#if USE_KHEAP
		if (fault_va >= KERNEL_HEAP_MAX)
			panic("Kernel: heap overflow exception!");
#endif
	}
	//2017: Check stack underflow for User
	else
	{
		//cprintf("trap from USER\n");
		if (fault_va >= USTACKTOP && fault_va < USER_TOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	//cprintf("curenv = %x\n", curenv);
	struct Env* faulted_env = cur_env;
	if (faulted_env == NULL)
	{
		cprintf("\nFaulted VA = %x\n", fault_va);
		print_trapframe(tf);
		panic("faulted env == NULL!");

	}
	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ( (faulted_env->env_page_directory[PDX(fault_va)] & PERM_PRESENT) != PERM_PRESENT)
	{
		faulted_env->tableFaultsCounter ++ ;
		table_fault_handler(faulted_env, fault_va);
	}
	else
	{
		if (userTrap)
				{
			fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
			int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
			        if(fault_va>=USER_HEAP_START&&fault_va <USER_HEAP_MAX)
			        {
			            if(((perms&PERM_UHPAGE) != PERM_UHPAGE) )
			                    {
			                        env_exit();
			                    }
			        }
			        if(fault_va >= USTACKTOP)
			        {
			            env_exit();
			        }
			        if(((perms&PERM_WRITEABLE)!=PERM_WRITEABLE)&&(perms&PERM_PRESENT))
			        {
			            env_exit();
			        }
				}

		/*2022: Check if fault due to Access Rights */
		int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
		if (perms & PERM_PRESENT)
			panic("Page @va=%x is exist! page fault due to violation of ACCESS RIGHTS\n", fault_va) ;
		/*============================================================================================*/
		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter ++ ;
//				cprintf("[%08s] user PAGE fault va %08x\n", faulted_env->prog_name, fault_va);
//				cprintf("\nPage working set BEFORE fault handler...\n");
//				env_page_ws_print(faulted_env);
		//int ffb = sys_calculate_free_frames();
		if(isBufferingEnabled())
		{
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		}
		else
		{
			page_fault_handler(faulted_env, fault_va);
		}

		//		cprintf("\nPage working set AFTER fault handler...\n");
		//		env_page_ws_print(faulted_env);
		//		int ffa = sys_calculate_free_frames();
		//		cprintf("fault handling @%x: difference in free frames (after - before = %d)\n", fault_va, ffa - ffb);
	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/
}


//=========================
// [2] TABLE FAULT HANDLER:
//=========================
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//=========================
// [3] PAGE FAULT HANDLER:
//=========================
/* Calculate the number of page faults according th the OPTIMAL replacement strategy
 * Given:
 * 	1. Initial Working Set List (that the process started with)
 * 	2. Max Working Set Size
 * 	3. Page References List (contains the stream of referenced VAs till the process finished)
 *
 * 	IMPORTANT: This function SHOULD NOT change any of the given lists
 */
int get_optimal_num_faults(struct WS_List*intial_list,int maxWSSize,struct PageRef_List*pageRference)
{
	struct WS_List jo;
	LIST_INIT(&jo);
	struct WorkingSetElement*eido;
	struct Env* p = get_cpu_proc();
	LIST_FOREACH_SAFE(eido,(intial_list),WorkingSetElement){
		struct WorkingSetElement*copy = env_page_ws_list_create_element(p, eido->virtual_address);
		LIST_INSERT_TAIL(&(jo),copy);
	}
	int num_of_faults =0;
	struct PageRefElement*ref;
	LIST_FOREACH(ref,pageRference){
		bool found =0;
		struct WorkingSetElement*ws;
		LIST_FOREACH(ws,&(jo)){
			if(ref->virtual_address ==ws->virtual_address)
			{
				found =1;
				break;
			}
		}

			if(found)
				continue;


			num_of_faults++;
			if(LIST_SIZE(&(jo)) < maxWSSize){
				struct WorkingSetElement*element = env_page_ws_list_create_element(p, ref->virtual_address);
			LIST_INSERT_TAIL(&(jo),element);
			continue;}

			int far = 0;
			uint32 victem = 0;
			LIST_FOREACH(ws,&jo){
				int position =0;
				int willuse =0;
				bool f=0;
				for(struct PageRefElement*next=ref;next!=NULL;next=LIST_NEXT(next) ){
					position++;
					if (ws->virtual_address == next->virtual_address) {
						willuse = position;
						f=1;
						break;
					}
			}
				if(!f){
					victem = ws->virtual_address;
					break;
				}
				if (willuse > far) {
					far = willuse;
					victem = ws->virtual_address;
				}
				}
			struct WorkingSetElement*e ;
			LIST_FOREACH(e,&(jo))
			{
				if (e->virtual_address == victem) {
					LIST_REMOVE(&(jo), e);
					break;
				}
			}
			struct WorkingSetElement*x = env_page_ws_list_create_element(p, ref->virtual_address);
			LIST_INSERT_TAIL(&(jo), x);
		}
	return num_of_faults;
	}


void page_fault_handler(struct Env * faulted_env, uint32 fault_va)
{
#if USE_KHEAP
	if (isPageReplacmentAlgorithmOPTIMAL()) {
				fault_va = ROUNDDOWN(fault_va,PAGE_SIZE);
				// Copy initial WS to active list if you are at first time
				if(faulted_env->first_fault == 0){
					struct WorkingSetElement * ws;
					LIST_FOREACH(ws,&(faulted_env->page_WS_list)){
						struct WorkingSetElement *copy =env_page_ws_list_create_element(faulted_env,ws->virtual_address);
						LIST_INSERT_TAIL(&(faulted_env->ActiveList),copy);
					}
					faulted_env->first_fault =1;
				}
				uint32 *ptr =NULL;
				get_page_table(faulted_env->env_page_directory,fault_va,&ptr);
				uint32 entry = ptr[PTX(fault_va)];
				bool active =0;
				uint32 *ptr2 =NULL;
				struct FrameInfo *frame =get_frame_info(faulted_env->env_page_directory,fault_va,&ptr2);
				//PERM = 0 BUT HAVE PHISCAL FRAME
				if((entry&PERM_PRESENT) ==0 && (frame!=NULL)){
					struct WorkingSetElement*all;
					pt_set_page_permissions(faulted_env->env_page_directory,fault_va,PERM_PRESENT,0);
					LIST_FOREACH(all,&(faulted_env->ActiveList)){
						if(all->virtual_address == fault_va){
							struct PageRefElement*ref = (struct PageRefElement*)kmalloc(sizeof(struct PageRefElement));
							ref->virtual_address =fault_va;
							LIST_INSERT_TAIL(&(faulted_env->referenceStreamList),ref);
							active=1;
							break;

						}

					}
				}
				else{
					struct FrameInfo* f =NULL;
					int x=allocate_frame(&f);
					int y=map_frame(faulted_env->env_page_directory,f,fault_va,PERM_WRITEABLE|PERM_USER);
					if (x != 0||y!=0)
						panic("kernel out memory");
					int r = pf_read_env_page(faulted_env, (uint32*) fault_va);

					if (r == E_PAGE_NOT_EXIST_IN_PF) {


						if (!((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)
								|| (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))){
							unmap_frame(faulted_env->env_page_directory, fault_va);
							env_exit();
						}
					}
				}
				if((LIST_SIZE(&(faulted_env->ActiveList)) == (faulted_env->page_WS_max_size))&&(active ==0)){
					struct WorkingSetElement*ptr;
					LIST_FOREACH(ptr,&(faulted_env->ActiveList)){
						pt_set_page_permissions(faulted_env->env_page_directory,ptr->virtual_address,0,PERM_PRESENT);
					}
					struct WorkingSetElement*element = LIST_FIRST(&(faulted_env->ActiveList));

					for(struct WorkingSetElement*next;element!=NULL;element =next)
					{
						next =LIST_NEXT(element);
						LIST_REMOVE(&(faulted_env->ActiveList),element);
					}
				}
				if(active ==0){
					struct WorkingSetElement*new = env_page_ws_list_create_element(faulted_env,fault_va);
					LIST_INSERT_TAIL(&(faulted_env->ActiveList),new);
					struct PageRefElement*x = (struct PageRefElement*)kmalloc(sizeof(struct PageRefElement));
					x->virtual_address =fault_va;
					LIST_INSERT_TAIL(&(faulted_env->referenceStreamList),x);
					}
				}

	else
	{
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(faulted_env->page_WS_list));
		if(wsSize < (faulted_env->page_WS_max_size))
		{
			//TODO: [PROJECT'25.GM#3] FAULT HANDLER I - #3 placement
			//Your code is here
			//Comment the following line
			//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
			uint32 va =ROUNDDOWN(fault_va,PAGE_SIZE);
			struct FrameInfo* f =NULL;
			int x=allocate_frame(&f);
			int y=map_frame(faulted_env->env_page_directory,f,va,PERM_WRITEABLE|PERM_USER);
	        if (x != 0||y!=0)
	        	panic("kernel out memory");
	        int r =pf_read_env_page(faulted_env,(uint32*)va);
	        if (r==E_PAGE_NOT_EXIST_IN_PF){
	        	if (!((va>=USER_HEAP_START&&va < USER_HEAP_MAX)|| (va >= USTACKBOTTOM  && va < USTACKTOP))){
	        		unmap_frame(faulted_env->env_page_directory,va);
	        		env_exit();
	        	}

	        }
	        struct WorkingSetElement* e=env_page_ws_list_create_element(faulted_env, va);
	        LIST_INSERT_TAIL(&(faulted_env->page_WS_list), e);
	        wsSize++;
	        if (wsSize == faulted_env->page_WS_max_size) {
	        	faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
	        } else {
	        	faulted_env->page_last_WS_element = NULL;
	        }

		}
		else
		{
			if (isPageReplacmentAlgorithmCLOCK())
			{
				//TODO: [PROJECT'25.IM#1] FAULT HANDLER II - #3 Clock Replacement
				//Your code is here
				//Comment the following line
				//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");
				struct WorkingSetElement* ptr=faulted_env->page_last_WS_element;
				struct WorkingSetElement* check =  ptr ;
				do{
					uint32 *abo_eid=NULL;
					get_page_table(faulted_env->env_page_directory,ptr->virtual_address,&abo_eid);
					uint32 entry=abo_eid[PTX(ptr->virtual_address)];
					if ((entry&PERM_USED)==0){
						if((entry & PERM_MODIFIED)==PERM_MODIFIED){
							uint32*ptr_table=NULL;
							struct FrameInfo *ptr1=get_frame_info(faulted_env->env_page_directory,ptr->virtual_address,&ptr_table);
							pf_update_env_page(faulted_env,ptr->virtual_address,ptr1);
						}
						break;
					}
					pt_set_page_permissions(faulted_env->env_page_directory, ptr->virtual_address, 0, PERM_USED);
					ptr=LIST_NEXT(ptr);
					if (ptr==NULL)
						ptr= LIST_FIRST(&(faulted_env->page_WS_list));

				}while(2==2);
				struct WorkingSetElement* victim =ptr;
				 while (victim!=check){
					 struct WorkingSetElement* next_check =LIST_NEXT(check);
					 uint32 check_va = check->virtual_address;
					 LIST_REMOVE(&(faulted_env->page_WS_list),check);
					 LIST_INSERT_TAIL(&(faulted_env->page_WS_list),check);
					 check=next_check;
				 }

				 env_page_ws_invalidate(faulted_env, ptr->virtual_address);
				 replace(faulted_env,fault_va);


			}

			else if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
			{
					//TODO: [PROJECT'25.IM#6] FAULT HANDLER II - #2 LRU Aging Replacement
						//Your code is here
						   //Comment the following line
					//				panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");
					if (faulted_env!= NULL)
					{
						struct WorkingSetElement *ptr = NULL;
						struct WorkingSetElement* f = LIST_FIRST(&(faulted_env->page_WS_list));
						uint32 va=f->virtual_address ;
						uint32 old=f->time_stamp;
						LIST_FOREACH(ptr, &faulted_env->page_WS_list)
						{
							if (old>ptr->time_stamp)
							{
								va=ptr->virtual_address;
								old = ptr->time_stamp;
							}
						}
						int ta = pt_get_page_permissions(faulted_env->env_page_directory, va);
						if ((ta&PERM_MODIFIED)==PERM_MODIFIED)
						{
							uint32 * ptr_table=NULL;
							struct FrameInfo * yy=get_frame_info(faulted_env->env_page_directory,va,&ptr_table);
							pf_update_env_page(faulted_env,va,yy);
						}
						env_page_ws_invalidate(faulted_env,va);
				        replace(faulted_env,fault_va);
					}
			}
			else if (isPageReplacmentAlgorithmModifiedCLOCK())
			{

				//TODO: [PROJECT'25.IM#6] FAULT HANDLER II - #3 Modified Clock Replacement
				//Your code is here
				//Comment the following line
				//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");
				struct WorkingSetElement* check = faulted_env->page_last_WS_element;
				struct WorkingSetElement* v = check;
				int c = 0;
				int w_m = faulted_env->page_WS_max_size - 1;
				int t2 = 0;
				do {
					uint32 va = v->virtual_address;
					int ta = pt_get_page_permissions(faulted_env->env_page_directory, va);
					if (t2 == 0) {
						if (!(ta & PERM_USED) && !(ta & PERM_MODIFIED)) {
							break;
						}
						if (c == w_m) {
							t2 = 1;
							c = 0;
							v = LIST_FIRST(&faulted_env->page_WS_list);
							continue;
						}
					}
					else {
						if (ta & PERM_USED) {
							pt_set_page_permissions(faulted_env->env_page_directory, va, 0, PERM_USED);
						}
						else {
							break;
						}
						if (c == w_m) {
							t2 = 0;
							c = 0;
							v = LIST_FIRST(&faulted_env->page_WS_list);
							continue;
						}
					}
					v = LIST_NEXT(v);
					c++;
				} while (1==1);
				uint32 v_va = v->virtual_address;
				int pe = pt_get_page_permissions(faulted_env->env_page_directory, v_va);
				if (pe & PERM_MODIFIED) {
					uint32* pt = NULL;
					struct FrameInfo* yy = get_frame_info(faulted_env->env_page_directory, v_va, &pt);
					pf_update_env_page(faulted_env, v_va, yy);
				}
				while (v != check) {
					struct WorkingSetElement* next_check = LIST_NEXT(check);
					LIST_REMOVE(&faulted_env->page_WS_list, check);
					LIST_INSERT_TAIL(&faulted_env->page_WS_list, check);
					check = next_check;
				}
				env_page_ws_invalidate(faulted_env, v_va);
				replace(faulted_env, fault_va);
				}
		}
	}
#endif
}


void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}
void replace(struct Env * faulted_env, uint32 fault_va){


	uint32 va =ROUNDDOWN(fault_va,PAGE_SIZE);
	struct FrameInfo* f =NULL;
	int x=allocate_frame(&f);
	int y=map_frame(faulted_env->env_page_directory,f,va,PERM_WRITEABLE|PERM_USER);
	if (x != 0||y!=0)
		panic("kernel out memory");
	int r =pf_read_env_page(faulted_env,(uint32*)va);
	if (r==E_PAGE_NOT_EXIST_IN_PF){
		if (!((va>=USER_HEAP_START&&va < USER_HEAP_MAX)|| (va >= USTACKBOTTOM  && va < USTACKTOP))){
			unmap_frame(faulted_env->env_page_directory,va);
			env_exit();
		}

	}
	struct WorkingSetElement* e = env_page_ws_list_create_element(faulted_env, fault_va);
	LIST_INSERT_TAIL(&faulted_env->page_WS_list,e);
	faulted_env->page_last_WS_element = LIST_FIRST(&faulted_env->page_WS_list);
}



