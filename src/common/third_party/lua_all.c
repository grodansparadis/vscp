/*
* all.c -- Lua core, libraries and interpreter in a single file
*/

#define MAKE_LIB

/* default is to build the full interpreter */
#ifndef MAKE_LIB
#ifndef MAKE_LUAC
#undef  MAKE_LUA
#define MAKE_LUA
#endif
#endif

#define luaall_c

/* core -- used by all */
#include "lua-5.4.3/lapi.c"
#include "lua-5.4.3/lcode.c"
#include "lua-5.4.3/ldebug.c"
#include "lua-5.4.3/ldo.c"
#include "lua-5.4.3/ldump.c"
#include "lua-5.4.3/lfunc.c"
#include "lua-5.4.3/lgc.c"
#include "lua-5.4.3/llex.c"
#include "lua-5.4.3/lmem.c"
#include "lua-5.4.3/lobject.c"
#include "lua-5.4.3/lopcodes.c"
#include "lua-5.4.3/lparser.c"
#include "lua-5.4.3/lstate.c"
#include "lua-5.4.3/lstring.c"
#include "lua-5.4.3/ltable.c"
#include "lua-5.4.3/ltm.c"
#include "lua-5.4.3/lundump.c"
#include "lua-5.4.3/lvm.c"
#include "lua-5.4.3/lzio.c"

#include "lua-5.4.3/lauxlib.c"
#include "lua-5.4.3/lbaselib.c"
#include "lua-5.4.3/ldblib.c"
#include "lua-5.4.3/liolib.c"
#include "lua-5.4.3/linit.c"
#include "lua-5.4.3/lmathlib.c"
#include "lua-5.4.3/loadlib.c"
#include "lua-5.4.3/loslib.c"
#include "lua-5.4.3/lstrlib.c"
#include "lua-5.4.3/ltablib.c"

#include "lua-5.4.3/lcorolib.c"   // AKHE
#include "lua-5.4.3/lutf8lib.c"   // AKHE
//#include "lua-5.4.3/linit.c"      // AKHE
#include "lua-5.4.3/lctype.c"

//#include "lua-5.4.3/lua.c"


// #include "lua-5.4.3/src/lctype.c"
// #include "lua-5.4.3/src/lapi.c"
// #include "lua-5.4.3/src/lcode.c"
// #include "lua-5.4.3/src/ldebug.c"
// #include "lua-5.4.3/src/ldo.c"
// #include "lua-5.4.3/src/ldump.c"
// #include "lua-5.4.3/src/lfunc.c"
// #include "lua-5.4.3/src/lgc.c"
// #include "lua-5.4.3/src/llex.c"
// #include "lua-5.4.3/src/lmem.c"
// #include "lua-5.4.3/src/lobject.c"
// #include "lua-5.4.3/src/lopcodes.c"
// #include "lua-5.4.3/src/lparser.c"
// #include "lua-5.4.3/src/lstate.c"
// #include "lua-5.4.3/src/lstring.c"
// #include "lua-5.4.3/src/ltable.c"
// #include "lua-5.4.3/src/ltm.c"
// #include "lua-5.4.3/src/lundump.c"
// #include "lua-5.4.3/src/lvm.c"
// #include "lua-5.4.3/src/lzio.c"
// #include "lua-5.4.3/src/lcorolib.c"   // AKHE
// #include "lua-5.4.3/src/lutf8lib.c"   // AKHE
// #include "lua-5.4.3/src/linit.c"      // AKHE

// /* auxiliary library -- used by all */
// #include "lua-5.4.3/src/lauxlib.c"

// /* standard library  -- not used by luac */
// #ifndef MAKE_LUAC
// #include "lua-5.4.3/src/lbaselib.c"
// #include "lua-5.4.3/src/ldblib.c"
// #include "lua-5.4.3/src/liolib.c"
// #include "lua-5.4.3/src/lmathlib.c"
// #include "lua-5.4.3/src/loadlib.c"
// #include "lua-5.4.3/src/loslib.c"
// #include "lua-5.4.3/src/lstrlib.c"
// #include "lua-5.4.3/src/ltablib.c"
// #endif

// /* lua */
// #ifdef MAKE_LUA
// #include "lua/src/linit.c"
// #include "lua/src/lua.c"
// #endif

// /* luac */
// #ifdef MAKE_LUAC
// #include "lua/src/print.c"
// #include "lua/src/luac.c"
// #endif
