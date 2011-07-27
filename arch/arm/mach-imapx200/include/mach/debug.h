#ifndef _DEBUG_H
#define _DEBUG_H
void print_mem(ulong level,char *data,ulong len,const char tag[], ...)
        __attribute__((format(printf, 4, 5)));

#endif

