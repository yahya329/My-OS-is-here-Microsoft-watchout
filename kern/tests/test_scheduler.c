#include <kern/proc/priority_manager.h>
#include <inc/assert.h>
#include <kern/proc/user_environment.h>
#include <kern/cmd/command_prompt.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/cpu/sched.h>
#include "../mem/memory_manager.h"


#define INSTANCES_NUMBER 10
#define TOTAL_TEST_VALUES 5
uint8 firstTimeTest = 1;
int prog_orders[TOTAL_TEST_VALUES+1][INSTANCES_NUMBER] = {0};
int env_count[TOTAL_TEST_VALUES] = {0};

void print_order(int prog_orders[][INSTANCES_NUMBER])
{
	for (int i = 0; i < TOTAL_TEST_VALUES; i++)
	{
		cprintf("\t[%d]: ", i);
		for (int j = 0; j < INSTANCES_NUMBER; j++)
		{
			if (prog_orders[i][j] == 0)
				break;
			cprintf("%d, ", prog_orders[i][j]);
		}
		cprintf("\n");
	}
}

int find_in_range(int env_id, int start, int count)
{
	int ret = -1;
	acquire_kspinlock(&ProcessQueues.qlock);
	{
		struct Env *env = NULL;
		int i = 0, end = start + count;

		//REVERSE LOOP ON EXIT LIST (to be the same as the queue order)
		int numOfExitEnvs = LIST_SIZE(&ProcessQueues.env_exit_queue);
		env = LIST_LAST(&ProcessQueues.env_exit_queue);

		cprintf("searching for envID %d starting from %d till %d\n", env_id, start, end);
		for (; i < numOfExitEnvs; env = LIST_PREV(env))
			//LIST_FOREACH_R(env, &env_exit_queue)
		{
			if (i < start)
			{
				i++;
				continue;
			}
			if (i >= end)
				//return -1;
				break;

			if (env_id == env->env_id)
			{
				ret = i;
				break;
			}
			i++;
		}
	}
	release_kspinlock(&ProcessQueues.qlock);
	return ret;
}


void test_bsd_nice_0()
{
	panic("not implemented");

}


void test_bsd_nice_1()
{
	panic("not implemented");
}

void test_bsd_nice_2()
{
	panic("not implemented");
}


void test_priorityRR_0()
{
	int numOfIncorrect = 0;

	if (firstTimeTest)
	{
		firstTimeTest = 0;
		int priority_values[] = {0, 2, 4, 6, 8};
		for (int i = 0; i < INSTANCES_NUMBER/2; i++)
		{
			struct Env *env ;
			if (i == 4)
			{
				env = env_create("priRR_fib_small", 500, 0, 0);
			}
			else
			{
				env = env_create("priRR_fib", 500, 0, 0);
			}
			int priority_index = i % TOTAL_TEST_VALUES;
			env_set_priority(env->env_id, priority_values[priority_index]);
			if (env == NULL)
				panic("Loading programs failed\n");
			if (env->page_WS_max_size != 500)
				panic("The program working set size is not correct\n");

			switch (priority_values[priority_index])
			{
			case 0:
				prog_orders[0][env_count[0]++] = env->env_id;
				break;
			case 2:
				prog_orders[1][env_count[1]++] = env->env_id;
				break;
			case 4:
				prog_orders[2][env_count[2]++] = env->env_id;
				break;
			case 6:
				prog_orders[3][env_count[3]++] = env->env_id;
				break;
			case 8:
				prog_orders[4][env_count[4]++] = env->env_id;
				break;
			}
			sched_new_env(env);
		}
		// print_order(prog_orders);
		cprintf_colored(TEXT_light_cyan, "\n> Running... (After all running programs finish, Run the same command again.)\n");
		execute_command("runall");
	}
	else
	{
		cprintf_colored(TEXT_light_cyan, "\n> Checking...\n");
		sched_print_all();
		// print_order(prog_orders);
		int start_idx = 0;
		for (int i = 0; i < TOTAL_TEST_VALUES; i++)
		{
			for (int j = 0; prog_orders[i][j] != 0; j++)
			{
				int exist = find_in_range(prog_orders[i][j], start_idx, env_count[i]);
				if (exist == -1)
				{
					//panic("The programs' order of finishing is not correct\n");
					cprintf_colored(TEXT_TESTERR_CLR, "The finish order of program [%d] is not correct\n", prog_orders[i][j]);
					numOfIncorrect++;
				}
			}
			start_idx += env_count[i];
		}
		firstTimeTest = 0;
	}

	int eval = 100 - numOfIncorrect * 100 / TOTAL_TEST_VALUES;
	cprintf_colored(TEXT_light_green, "\ntest_priorityRR_0 is finished. Eval = %d%\n", eval);
}

void test_priorityRR_1()
{
	int numOfIncorrect = 0;

	if (firstTimeTest)
	{
		rsttst();
		firstTimeTest = 0;
		struct Env *fibPri0Env = env_create("priRR_fib", 500, 0, 0);
		struct Env *fibPri4Env = env_create("priRR_fib_pri4", 500, 0, 0);
		struct Env *fibPri8Env = env_create("priRR_fib_pri8", 500, 0, 0);
		struct Env *fibPri2ParentEnv = env_create("priRR_fib_create", 500, 0, 0);
		if (fibPri0Env == NULL || fibPri4Env == NULL || fibPri8Env == NULL || fibPri2ParentEnv == NULL)
			panic("Loading programs failed\n");
		if (fibPri0Env->page_WS_max_size != 500 || fibPri4Env->page_WS_max_size != 500 || fibPri8Env->page_WS_max_size != 500 || fibPri2ParentEnv->page_WS_max_size != 500)
			panic("The programs should be initially loaded with the given working set size.\n");
		sched_new_env(fibPri8Env);
		sched_new_env(fibPri0Env);
		sched_new_env(fibPri4Env);
		sched_new_env(fibPri2ParentEnv);
		env_set_priority(fibPri2ParentEnv->env_id, 4);

		prog_orders[0][0] = fibPri0Env->env_id;
		prog_orders[1][0] = fibPri2ParentEnv->env_id ; //id of the parent
		prog_orders[2][0] = fibPri2ParentEnv->env_id + 1; //id of the 1st created child fib
		prog_orders[3][0] = fibPri4Env->env_id;
		prog_orders[4][0] = fibPri8Env->env_id;
		prog_orders[5][0] = fibPri2ParentEnv->env_id + 2; //id of the 2nd created child fib

		cprintf_colored(TEXT_light_cyan, "\n> Running... (After all running programs finish, Run the same command again.)\n");
		execute_command("runall");
	}
	else
	{
		cprintf_colored(TEXT_light_cyan, "\n> Checking...\n");
		sched_print_all();
		// print_order(prog_orders);
		int i = 0;
		struct Env *env = NULL;
		acquire_kspinlock(&ProcessQueues.qlock);
		{
			//REVERSE LOOP ON EXIT LIST (to be the same as the queue order)
			int numOfExitEnvs = LIST_SIZE(&ProcessQueues.env_exit_queue);
			env = LIST_LAST(&ProcessQueues.env_exit_queue);
			for (; i < numOfExitEnvs; env = LIST_PREV(env))
				//LIST_FOREACH_R(env, &env_exit_queue)
			{
				//cprintf("%s - id=%d, priority=%d\n", env->prog_name, env->env_id, env->priority);
				if (prog_orders[i][0] != env->env_id)
				{
					cprintf_colored(TEXT_TESTERR_CLR, "The finish order of program [%d - %s] is not correct\n", env->env_id, env->prog_name);
					numOfIncorrect++;
				}
				i++;
			}
		}
		release_kspinlock(&ProcessQueues.qlock);
	}
	int eval = 100 - numOfIncorrect * 100 / 6;
	cprintf_colored(TEXT_light_green, "\ntest_priorityRR_1 is finished. Eval = %d%\n", eval);
}

void test_priorityRR_2()
{
	int numOfIncorrect = 0;
	int totalNumOfProcesses = 0;
	if (firstTimeTest)
	{
		firstTimeTest = 0;
		int priority_values[] = {0, 2, 4, 6};
		for (int i = 0; i < INSTANCES_NUMBER - 2; i++)
		{
			struct Env *env = env_create("priRR_fib", 500, 0, 0);
			int priority_index = i % (TOTAL_TEST_VALUES-1);
			env_set_priority(env->env_id, priority_values[priority_index]);
			if (env == NULL)
				panic("Loading programs failed\n");
			if (env->page_WS_max_size != 500)
				panic("The program working set size is not correct\n");

			switch (priority_values[priority_index])
			{
			case 0:
				prog_orders[1][env_count[1]++] = env->env_id;
				break;
			case 2:
				prog_orders[2][env_count[2]++] = env->env_id;
				break;
			case 4:
				prog_orders[3][env_count[3]++] = env->env_id;
				break;
			case 6:
				prog_orders[4][env_count[4]++] = env->env_id;
				break;
			}
			sched_new_env(env);
		}

		int priority_values2[] = {0, 1, 2, 3, 4, 5, 6, 7 };
		for (int i = 0; i < INSTANCES_NUMBER - 2; i++)
		{
			struct Env *env = env_create("priRR_fib_small", 500, 0, 0);
			int priority_index = i ;
			env_set_priority(env->env_id, priority_values2[priority_index]);
			if (env == NULL)
				panic("Loading programs failed\n");
			if (env->page_WS_max_size != 500)
				panic("The program working set size is not correct\n");

			prog_orders[0][env_count[0]++] = env->env_id;

			sched_new_env(env);
		}
		// print_order(prog_orders);
		cprintf_colored(TEXT_light_cyan, "\n> Running... (After all running programs finish, Run the same command again.)\n");
		execute_command("runall");
	}
	else
	{
		cprintf_colored(TEXT_light_cyan, "\n> Checking...\n");
		sched_print_all();
		// print_order(prog_orders);
		int start_idx = 0;
		for (int i = 0; i < TOTAL_TEST_VALUES; i++)
		{
			if (i == 0) //small programs should finish in their strict order
			{
				for (int j = 0; j < INSTANCES_NUMBER - 2; j++)
				{
					totalNumOfProcesses++;
					int exist = find_in_range(prog_orders[i][j], j, 1);
					if (exist == -1)
					{
						//panic("The programs' order of finishing is not correct\n");
						cprintf_colored(TEXT_TESTERR_CLR, "The finish order of program [%d] is not correct\n", prog_orders[i][j]);
						numOfIncorrect++;
					}
				}
				start_idx += env_count[i];
			}
			else
			{
				for (int j = 0; prog_orders[i][j] != 0; j++)
				{
					totalNumOfProcesses++;
					int exist = find_in_range(prog_orders[i][j], start_idx, env_count[i]);
					if (exist == -1)
					{
						//panic("The programs' order of finishing is not correct\n");
						cprintf_colored(TEXT_TESTERR_CLR, "The finish order of program [%d] is not correct\n", prog_orders[i][j]);
						numOfIncorrect++;
					}
				}
				start_idx += env_count[i];
			}
		}
		firstTimeTest = 0;
	}
	int eval = 100 - numOfIncorrect * 100 / totalNumOfProcesses;
	cprintf("totalNumOfProcesses = %d\n ", totalNumOfProcesses);
	cprintf_colored(TEXT_light_green, "\ntest_priorityRR_2 is finished. Eval = %d%\n", eval);
}
