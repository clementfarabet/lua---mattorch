
#define LIBNAME "mattorch"

#include <lua.h>
#include <luaT.h>
#include <TH.h>
#include <lualib.h>
#include <lauxlib.h>
#include <mat.h>

// initialize Lua stack/state, should always be called first
void mattorch_init(void);

// close Lua state, collect all garbage, should be called last
void mattorch_close(void);

// execute/parse Lua file, that file should define global
// functions to be called by mattorch_callfunc()
int mattorch_dofile(const char *file);

// same as above, but parse string directly
int mattorch_dostring(const char *string);

/* require a library */
int mattorch_dorequire(const char *name);

// call any Lua function that exists in the global Lua namespace (_G)
// the function takes NINPUTS input matrices, and should return 
// NOUTPUTS output matrices. NINPUTS/NOUTPUTS should match what the
// Lua function expects.
// Matlab matrices of all types are supported and converted to
// torch.Tensors() automatically.
mxArray ** mattorch_callfunc(const char *funcname, 
                             int ninputs, int noutputs, 
                             mxArray **inputs);
