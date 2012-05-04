
// Support provided to experiments.

#ifndef __MAIN_H
#define __MAIN_H


#include "stdbool.h"
#include "core.h"


#define  NULL  0


#define  ITEMS_IN_ARRAY( a)  ( sizeof(a) / sizeof(a[0]) )


extern bool volatile  should_run;

extern VoidFunction *counter3_overflow_hook;


extern
void db( char *message)
;


#endif

