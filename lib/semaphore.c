// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	panic("create_semaphore() is not implemented yet...!!");
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	panic("get_semaphore() is not implemented yet...!!");
}

void wait_semaphore(struct semaphore sem)
{
	panic("wait_semaphore() is not implemented yet...!!");
}

void signal_semaphore(struct semaphore sem)
{
	panic("signal_semaphore() is not implemented yet...!!");
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
