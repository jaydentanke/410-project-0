#include <stdio.h>
#include <unistd.h>
#include "traceback.h"

void bar(int x, int y)
{
  int z;
  z = x + y;
  (void) z;
  traceback(STDOUT_FILENO);
}

void foo() {
  bar (5,17);
}

int main (int argc, char **argv)
{
  foo();
  return 0;
}
