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
#include "lua/src/lctype.c"
#include "lua/src/lapi.c"
#include "lua/src/lcode.c"
#include "lua/src/ldebug.c"
#include "lua/src/ldo.c"
#include "lua/src/ldump.c"
#include "lua/src/lfunc.c"
#include "lua/src/lgc.c"
#include "lua/src/llex.c"
#include "lua/src/lmem.c"
#include "lua/src/lobject.c"
#include "lua/src/lopcodes.c"
#include "lua/src/lparser.c"
#include "lua/src/lstate.c"
#include "lua/src/lstring.c"
#include "lua/src/ltable.c"
#include "lua/src/ltm.c"
#include "lua/src/lundump.c"
#include "lua/src/lvm.c"
#include "lua/src/lzio.c"
#include "lua/src/lcorolib.c"   // AKHE
#include "lua/src/lutf8lib.c"   // AKHE
#include "lua/src/linit.c"      // AKHE

/* auxiliary library -- used by all */
#include "lua/src/lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#include "lua/src/lbaselib.c"
#include "lua/src/ldblib.c"
#include "lua/src/liolib.c"
#include "lua/src/lmathlib.c"
#include "lua/src/loadlib.c"
#include "lua/src/loslib.c"
#include "lua/src/lstrlib.c"
#include "lua/src/ltablib.c"
#endif

/* lua */
#ifdef MAKE_LUA
#include "lua/src/linit.c"
#include "lua/src/lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#include "lua/src/print.c"
#include "lua/src/luac.c"
#endif
