
#ifndef __CORE_H
#define __CORE_H


#ifndef NULL
#define  NULL  0
#endif


#define  ITEMS_IN_ARRAY( a)  ( sizeof(a) / sizeof(a[0]) )


typedef void VoidFunction();


#ifndef __EMULATED
typedef  int  size_t;

extern
size_t strlen( char const *s)
;
#endif


#endif

