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
#include "lua-5.3.5/src/lctype.c"
#include "lua-5.3.5/src/lapi.c"
#include "lua-5.3.5/src/lcode.c"
#include "lua-5.3.5/src/ldebug.c"
#include "lua-5.3.5/src/ldo.c"
#include "lua-5.3.5/src/ldump.c"
#include "lua-5.3.5/src/lfunc.c"
#include "lua-5.3.5/src/lgc.c"
#include "lua-5.3.5/src/llex.c"
#include "lua-5.3.5/src/lmem.c"
#include "lua-5.3.5/src/lobject.c"
#include "lua-5.3.5/src/lopcodes.c"
#include "lua-5.3.5/src/lparser.c"
#include "lua-5.3.5/src/lstate.c"
#include "lua-5.3.5/src/lstring.c"
#include "lua-5.3.5/src/ltable.c"
#include "lua-5.3.5/src/ltm.c"
#include "lua-5.3.5/src/lundump.c"
#include "lua-5.3.5/src/lvm.c"
#include "lua-5.3.5/src/lzio.c"
#include "lua-5.3.5/src/lcorolib.c"   // AKHE
#include "lua-5.3.5/src/lutf8lib.c"   // AKHE
#include "lua-5.3.5/src/linit.c"      // AKHE

/* auxiliary library -- used by all */
#include "lua-5.3.5/src/lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#include "lua-5.3.5/src/lbaselib.c"
#include "lua-5.3.5/src/ldblib.c"
#include "lua-5.3.5/src/liolib.c"
#include "lua-5.3.5/src/lmathlib.c"
#include "lua-5.3.5/src/loadlib.c"
#include "lua-5.3.5/src/loslib.c"
#include "lua-5.3.5/src/lstrlib.c"
#include "lua-5.3.5/src/ltablib.c"
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
