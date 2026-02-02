// Called from entry.S to get us going.
// entry.S already took care of defining envs, pages, vpd, and vpt.

#include <inc/lib.h>

extern void _main(int argc, char **argv);

volatile struct Env *myEnv = NULL;
volatile bool printStats = 1;

volatile char *binaryname = "(PROGRAM NAME UNKNOWN)";
void
libmain(int argc, char **argv)
{
	//printStats = 1;
	int envIndex = sys_getenvindex();

	myEnv = &(envs[envIndex]);

	//SET THE PROGRAM NAME
	if (myEnv->prog_name[0] != '\0')
		binaryname = myEnv->prog_name;

	// set env to point at our env structure in envs[].
	// env = envs;

	// save the name of the program so that panic() can use it
	if (argc > 0)
		binaryname = argv[0];

	// call user main routine
	_main(argc, argv);

	if (printStats)
	{
		char isOPTReplCmd[100] = "__IsOPTRepl__" ;
		int isOPTRepl = 0;
		sys_utilities(isOPTReplCmd, (uint32)(&isOPTRepl));

		sys_lock_cons();
		{
			cprintf("**************************************\n");
			if (isOPTRepl)
			{
				cprintf("OPTIMAL number of page faults = %d\n", sys_get_optimal_num_faults());
			}
			else
			{
				cprintf("Num of PAGE faults = %d, modif = %d\n", myEnv->pageFaultsCounter, myEnv->nModifiedPages);
				cprintf("# PAGE IN (from disk) = %d, # PAGE OUT (on disk) = %d, # NEW PAGE ADDED (on disk) = %d\n", myEnv->nPageIn, myEnv->nPageOut,myEnv->nNewPageAdded);
			}
			//cprintf("Num of freeing scarce memory = %d, freeing full working set = %d\n", myEnv->freeingScarceMemCounter, myEnv->freeingFullWSCounter);
			cprintf("Num of clocks = %d\n", myEnv->nClocks);
			cprintf("**************************************\n");
		}
		sys_unlock_cons();
	}

	// exit gracefully
	exit();
}

