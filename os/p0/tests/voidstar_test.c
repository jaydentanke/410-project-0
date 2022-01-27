#include <stdio.h>
#include <unistd.h>
#include "traceback.h"


void foo(void *lookma);
void bar(int x, int y);


void foo(void *lookma) {
  bar (5,17);
}

void bar(int x, int y)
{
  int z;
  z = x + y;
  (void) z;
  traceback(STDOUT_FILENO);
}

int main (int argc, char **argv)
{
  foo( (void*) &argc ); // Avoid "is NULL 0x0 or (nil)?" issue
  return 0;
}
