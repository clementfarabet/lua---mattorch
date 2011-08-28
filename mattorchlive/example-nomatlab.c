
#include "mattorchlive.h"

int main(void)
{
  /* init Lua state */
  mattorch_init();

  /* parse Lua code */
  mattorch_dofile("example.lua");
}
