
#include "mex.h"
#include "mattorchlive.h"

static int firstcall = 1;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) 
{
  /* first call ? */
  if (firstcall) {
    /* init Lua state */
    mattorch_init();
    firstcall = 0;

    /* parse Lua code */
    mattorch_dofile("example.lua");
  }

  /* 1 arg required */
  if (nlhs < 1) {
    printf("at least one arg required\n");
    return;
  }

  /* call a function defined in example.lua, with one input, one output */
  mxArray *inputs[1];
  inputs[1] = plhs[0];
  mxArray **outputs = mattorch_callfunc("transpose", 1, 1, inputs);

  /* return result */
  if (nrhs >= 1) {
    prhs[0] = outputs[0];
  }
}
