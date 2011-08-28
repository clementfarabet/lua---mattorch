
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "mattorchlive.h"

static lua_State *L = NULL;
static const char *progname = "lua";

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(L, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}

static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

void mattorch_init(void) {
  /* Set CWD before starting Lua */
  lua_executable_dir("./lua");

  /* Declare a Lua State, open the Lua State and load all libraries */
  L = lua_open();
  lua_gc(L, LUA_GCSTOP, 0);
  luaL_openlibs(L);
  lua_gc(L, LUA_GCRESTART, 0);
}

void mattorch_close(void) {
  /* Destroy the Lua State */
  lua_close(L);
}

int mattorch_dofile(const char *name)
{
  /* Load user file */
  int err = luaL_loadfile(L, name) || docall(L, 0, 1);

  /* Error ? */
  if (err) {
    printf("<%s> ERROR: %s could not be loaded\n", LIBNAME, name);
  }
  return report(L, err);
}

int mattorch_dostring(const char *s)
{
  /* Load user file */
  int err = luaL_loadbuffer(L, s, strlen(s), "name") || docall(L, 0, 1);

  /* Error ? */
  if (err) {
    printf("<%s> ERROR: could not parse string\n", LIBNAME);
  }
  return report(L, err);
}

int mattorch_dorequire(const char *name)
{
  /* Load library */
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  int err = docall(L, 1, 1);

  /* Error ? */
  if (err) {
    printf("<%s> ERROR: could not require Library\n", LIBNAME);
  }
  return report(L, err);
}

mxArray ** mattorch_callfunc(const char *funcname, int ninputs, int noutputs, const mxArray **inputs)
{
  // (1) push function on top of stack
  lua_getfield(L, LUA_GLOBALSINDEX, funcname);

  // (2) convert all incoming matrices -> tensors
  int i;
  for (i=0; i<ninputs; i++) {

    // get single matrix
    const mxArray *pa = inputs[i];

    // get dimensions
    mwSize ndims = mxGetNumberOfDimensions(pa);
    const mwSize *dims = mxGetDimensions(pa);

    // infer size and stride
    int k;
    THLongStorage *size = THLongStorage_newWithSize(ndims);
    THLongStorage *stride = THLongStorage_newWithSize(ndims);
    for (k=0; k<ndims; k++) {
      THLongStorage_set(size, ndims-k-1, dims[k]);
      if (k > 0)
        THLongStorage_set(stride, ndims-k-1, dims[k-1]*THLongStorage_get(stride,ndims-k));
      else
        THLongStorage_set(stride, ndims-k-1, 1);
    }

    if (mxGetClassID(pa) == mxDOUBLE_CLASS) {
      THDoubleTensor *tensor = THDoubleTensor_newWithSize(size, stride);
      memcpy((void *)(THDoubleTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THDoubleTensor_nElement(tensor) * sizeof(double));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.DoubleTensor"));

    } else if (mxGetClassID(pa) == mxSINGLE_CLASS) {
      THFloatTensor *tensor = THFloatTensor_newWithSize(size, stride);
      memcpy((void *)(THFloatTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THFloatTensor_nElement(tensor) * sizeof(float));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.FloatTensor"));

    } else if (mxGetClassID(pa) == mxINT64_CLASS) {
      THLongTensor *tensor = THLongTensor_newWithSize(size, stride);
      memcpy((void *)(THLongTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THLongTensor_nElement(tensor) * sizeof(long));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.LongTensor"));

    } else if (mxGetClassID(pa) == mxINT32_CLASS) {
      THIntTensor *tensor = THIntTensor_newWithSize(size, stride);
      memcpy((void *)(THIntTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THIntTensor_nElement(tensor) * sizeof(int));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.IntTensor"));

    } else if (mxGetClassID(pa) == mxUINT32_CLASS) {
      THIntTensor *tensor = THIntTensor_newWithSize(size, stride);
      memcpy((void *)(THIntTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THIntTensor_nElement(tensor) * sizeof(int));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.IntTensor"));

    } else if ((mxGetClassID(pa) == mxINT16_CLASS)) {
      THShortTensor *tensor = THShortTensor_newWithSize(size, stride);
      memcpy((void *)(THShortTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THShortTensor_nElement(tensor) * sizeof(short));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ShortTensor"));

    } else if ((mxGetClassID(pa) == mxUINT16_CLASS)) {
      THShortTensor *tensor = THShortTensor_newWithSize(size, stride);
      memcpy((void *)(THShortTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THShortTensor_nElement(tensor) * sizeof(short));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ShortTensor"));

    } else if ((mxGetClassID(pa) == mxINT8_CLASS) || (mxGetClassID(pa) == mxCHAR_CLASS)) {
      THCharTensor *tensor = THCharTensor_newWithSize(size, stride);
      memcpy((void *)(THCharTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THCharTensor_nElement(tensor) * sizeof(char));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.CharTensor"));

    } else if ((mxGetClassID(pa) == mxUINT8_CLASS)) {
      THByteTensor *tensor = THByteTensor_newWithSize(size, stride);
      memcpy((void *)(THByteTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THByteTensor_nElement(tensor) * sizeof(char));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ByteTensor"));

    } else if ((mxGetClassID(pa) == mxLOGICAL_CLASS)) {
      THByteTensor *tensor = THByteTensor_newWithSize(size, stride);
      memcpy((void *)(THByteTensor_data(tensor)),
             (void *)(mxGetPr(pa)), THByteTensor_nElement(tensor) * sizeof(char));
      luaT_pushudata(L, tensor, luaT_checktypename2id(L, "torch.ByteTensor"));

    } else {
      THError("unsupported Matlab type");
    }
  }

  // (3) now that all the args are pushed on the stack,
  //     make the function call
  lua_call(L, ninputs, noutputs);

  // (4) retrieve all results from function
  mxArray **outputs = malloc(sizeof(mxArray *) * noutputs);
  int o;
  for (o=0; o<noutputs; o++) {

    // retrieve tensors returned, depending on their type
    if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.DoubleTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.DoubleTensor"));
      THDoubleTensor *tensorc = THDoubleTensor_newContiguous((THDoubleTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxDOUBLE_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THDoubleTensor_data(tensorc)),
             THDoubleTensor_nElement(tensorc) * sizeof(double));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.FloatTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.FloatTensor"));
      THFloatTensor *tensorc = THFloatTensor_newContiguous((THFloatTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxSINGLE_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THFloatTensor_data(tensorc)),
             THFloatTensor_nElement(tensorc) * sizeof(float));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.IntTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.IntTensor"));
      THIntTensor *tensorc = THIntTensor_newContiguous((THIntTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxINT32_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THIntTensor_data(tensorc)),
             THIntTensor_nElement(tensorc) * sizeof(int));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.LongTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.LongTensor"));
      THLongTensor *tensorc = THLongTensor_newContiguous((THLongTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxINT64_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THLongTensor_data(tensorc)),
             THLongTensor_nElement(tensorc) * sizeof(long));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.ShortTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.ShortTensor"));
      THShortTensor *tensorc = THShortTensor_newContiguous((THShortTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxINT16_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THShortTensor_data(tensorc)),
             THShortTensor_nElement(tensorc) * sizeof(short));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.CharTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.CharTensor"));
      THCharTensor *tensorc = THCharTensor_newContiguous((THCharTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxINT8_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)), 
             (void *)(THCharTensor_data(tensorc)),
             THCharTensor_nElement(tensorc) * sizeof(char));
      outputs[o] = pm;

    } else if (luaT_isudata(L, -1-o, luaT_checktypename2id(L, "torch.ByteTensor"))) {
      // export tensor into matrix
      void *tensor = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.ByteTensor"));
      THByteTensor *tensorc = THByteTensor_newContiguous((THByteTensor *)tensor);
      mwSize size[] = {-1,-1,-1,-1,-1,-1,-1,-1};
      const long ndims = tensorc->nDimension;
      int k;
      for (k=0; k<ndims; k++) size[k] = tensorc->size[ndims-k-1];
      mxArray *pm = mxCreateNumericArray(ndims, size, mxUINT8_CLASS, mxREAL);
      memcpy((void *)(mxGetPr(pm)),
             (void *)(THByteTensor_data(tensorc)),
             THByteTensor_nElement(tensorc) * sizeof(char));
      outputs[o] = pm;

    } else {
      THError("unsupported Matlab type");
    }
  }
  lua_pop(L, noutputs);

  // return outputs
  return outputs;
}
