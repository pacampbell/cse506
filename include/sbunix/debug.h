#ifndef __DEBUG_H
#define __DEBUG_H
	#define BOCHS_MAGIC() do { 			\
		__asm__ __volatile__(			\
			"xchg %%bx, %%bx;"			\
			:							\
			:							\
			:							\
			); 							\
	} while(0)
#endif