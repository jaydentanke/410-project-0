#ifndef __TRACEBACK_HELPERS_H
#define __TRACEBACK_HELPERS_H

void* get_ebp();
void* get_function_addr_from_ebp(void* bp, void* next_bp);

#endif /* __TRACEBACK_HELPERS_H */