// Simple implementation of cprintf console output for the kernel,
// based on printfmt() and the kernel console's cputchar().

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/stdarg.h>
#include <kern/cpu/cpu.h>
#include <kern/cons/console.h>


static void
putch(int ch, int *cnt)
{
	cputchar(ch);
	(*cnt)++;
}

int
vcprintf(const char *fmt, va_list ap)
{
	int cnt = 0;

	vprintfmt((void*)putch, &cnt, fmt, ap);
	return cnt;
}

int
cprintf(const char *fmt, ...)
{
	int cnt;
	pushcli();	//disable interrupts
	{
		va_list ap;

		va_start(ap, fmt);
		cnt = vcprintf(fmt, ap);
		va_end(ap);
	}
	popcli();	//enable interrupts

	return cnt;
}

// *************** This text coloring feature is implemented by *************
// ********** Abd-Alrahman Zedan From Team Frozen-Bytes - FCIS'24-25 ********
int
cprintf_colored(int textClr, const char *fmt, ...)
{
	current_text_color = (textClr << 8) ;
	int cnt;
	pushcli();	//disable interrupts
	{
		va_list ap;

		va_start(ap, fmt);
		cnt = vcprintf(fmt, ap);
		va_end(ap);
	}
	popcli();	//enable interrupts
	current_text_color = TEXT_DEFAULT_CLR; //restore default text color

	return cnt;
}
