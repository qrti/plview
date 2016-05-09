#ifndef _GC_H_
#define _GC_H_

void gc_handler(int);
void gc_attach(int (*fp)(void));
void gc_catch(void);
int gc_run(void);

#endif
