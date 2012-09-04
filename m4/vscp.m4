dnl  Macros needed for VSCP
dnl  $Id: vscp.m4,v 1.1 2005/09/05 20:02:18 akhe Exp $

dnl ===========================================================================
dnl  Macros to add plugins or builtins and handle their flags

AC_DEFUN([VSCP_ADD_BUILTINS], [
  BUILTINS="${BUILTINS} $1"
])

AC_DEFUN([VSCP_ADD_PLUGINS], [
  PLUGINS="${PLUGINS} $1"
])

dnl  Special cases: VSCP, pics, plugins, save
AC_DEFUN([VSCP_ADD_CPPFLAGS], [
  for element in [$1]; do
    eval "CPPFLAGS_${element}="'"$'"{CPPFLAGS_${element}} $2"'"'
    am_modules_with_cppflags="${am_modules_with_cppflags} ${element}"
  done
])

AC_DEFUN([VSCP_ADD_CFLAGS], [
  for element in [$1]; do
    eval "CFLAGS_${element}="'"$'"{CFLAGS_${element}} $2"'"'
    am_modules_with_cflags="${am_modules_with_cflags} ${element}"
  done
])

AC_DEFUN([VSCP_ADD_CXXFLAGS], [
  for element in [$1]; do
    eval "CXXFLAGS_${element}="'"$'"{CXXFLAGS_${element}} $2"'"'
    am_modules_with_cxxflags="${am_modules_with_cxxflags} ${element}"
  done
])

AC_DEFUN([VSCP_ADD_OBJCFLAGS], [
  for element in [$1]; do
    eval "OBJCFLAGS_${element}="'"$'"{OBJCFLAGS_${element}} $2"'"'
    am_modules_with_objcflags="${am_modules_with_objcflags} ${element}"
  done
])

AC_DEFUN([VSCP_ADD_LDFLAGS], [
  for element in [$1]; do
    eval "LDFLAGS_${element}="'"'"$2 "'$'"{LDFLAGS_${element}} "'"'
    am_modules_with_ldflags="${am_modules_with_ldflags} ${element}"
  done
])

dnl ===========================================================================
dnl  Macros to save and restore default flags

AC_DEFUN([VSCP_SAVE_FLAGS], [
  CPPFLAGS_save="${CPPFLAGS}"
  CFLAGS_save="${CFLAGS}"
  CXXFLAGS_save="${CXXFLAGS}"
  OBJCFLAGS_save="${OBJCFLAGS}"
  LDFLAGS_save="${LDFLAGS}"
])

AC_DEFUN([VSCP_RESTORE_FLAGS], [
  CPPFLAGS="${CPPFLAGS_save}"
  CFLAGS="${CFLAGS_save}"
  CXXFLAGS="${CXXFLAGS_save}"
  OBJCFLAGS="${OBJCFLAGS_save}"
  LDFLAGS="${LDFLAGS_save}"
])

dnl ===========================================================================
dnl  Helper macro to generate the vscp-config.in file

AC_DEFUN([VSCP_OUTPUT_VSCP_CONFIG_IN], [

  AC_MSG_RESULT(configure: creating ./vscp-config.in)

  am_all_modules="`for x in ${am_modules_with_cppflags} ${am_modules_with_cflags} ${am_modules_with_cxxflags} ${am_modules_with_objcflags} ${am_modules_with_ldflags}; do echo $x; done | sort | uniq`"

  rm -f vscp-config.in
  sed -ne '/#@1@#/q;p' < "${srcdir}/vscp-config.in.in" \
    | sed -e "s/@gprof@/${enable_gprof}/" \
          -e "s/@cprof@/${enable_cprof}/" \
          -e "s/@optim@/${enable_optimizations}/" \
          -e "s/@debug@/${enable_debug}/" \
          -e "s/@release@/${enable_release}/" \
          -e "s/@PLUGINS@/${PLUGINS}/" \
          -e "s/@BUILTINS@/${BUILTINS}/" \
          -e "s/@CFLAGS_TUNING@/${CFLAGS_TUNING}/" \
          -e "s/@CFLAGS_OPTIM@/${CFLAGS_OPTIM}/" \
          -e "s/@CFLAGS_OPTIM_NODEBUG@/${CFLAGS_OPTIM_NODEBUG}/" \
          -e "s/@CFLAGS_NOOPTIM@/${CFLAGS_NOOPTIM}/" \
    > vscp-config.in

  dnl  Switch/case loop
  for x in `echo ${am_all_modules}`
  do [
    echo "    ${x})" >> vscp-config.in
    if test "`eval echo @'$'CPPFLAGS_${x}@`" != "@@"; then
      echo "      cppflags=\"\${cppflags} `eval echo '$'CPPFLAGS_${x}`\"" >> vscp-config.in
    fi
    if test "`eval echo @'$'CFLAGS_${x}@`" != "@@"; then
      echo "      cflags=\"\${cflags} `eval echo '$'CFLAGS_${x}`\"" >> vscp-config.in
    fi
    if test "`eval echo @'$'CXXFLAGS_${x}@`" != "@@"; then
      echo "      cxxflags=\"\${cxxflags} `eval echo '$'CXXFLAGS_${x}`\"" >> vscp-config.in
      if test "${x}" != "plugin" -a "${x}" != "builtin"; then
        echo "      linkage=\"c++\"" >> vscp-config.in
      fi
    fi
    if test "`eval echo @'$'OBJCFLAGS_${x}@`" != "@@"; then
      echo "      objcflags=\"\${objcflags} `eval echo '$'OBJCFLAGS_${x}`\"" >> vscp-config.in
      if test "${x}" != "plugin" -a "${x}" != "builtin"; then
        echo "      if test \"\${linkage}\" = \"c\"; then linkage=\"objc\"; fi" >> vscp-config.in
      fi
    fi
    if test "`eval echo @'$'LDFLAGS_${x}@`" != "@@"; then
      echo "      ldflags=\"\${ldflags} `eval echo '$'LDFLAGS_${x}`\"" >> vscp-config.in
    fi
    echo "    ;;" >> vscp-config.in
  ] done

  dnl  '/#@1@#/,/#@2@#/{/#@.@#/d;p}' won't work on OS X
  sed -ne '/#@1@#/,/#@2@#/p' < "${srcdir}/vscp-config.in.in" \
   | sed -e '/#@.@#/d' >> vscp-config.in

  VSCP_CONFIG_HELPER

  dnl  '/#@2@#/,${/#@.@#/d;p}' won't work on OS X
  sed -ne '/#@2@#/,$p' < "${srcdir}/vscp-config.in.in" \
   | sed -e '/#@.@#/d' >> vscp-config.in
])

dnl ===========================================================================
dnl  Macros for shared object handling (TODO)

AC_DEFUN([VSCP_LIBRARY_SUFFIX], [
  AC_MSG_CHECKING(for shared objects suffix)
  case "${target_os}" in
    darwin*)
      LIBEXT=".dylib"
      ;;
    *mingw32* | *cygwin* | *wince* | *mingwce* | *pe*)
      LIBEXT=".dll"
      ;;
    hpux*)
      LIBEXT=".sl"
      ;;
    *)
      LIBEXT=".so"
      ;;
  esac
  AC_MSG_RESULT(${LIBEXT})
  AC_DEFINE_UNQUOTED(LIBEXT, "${LIBEXT}", [Dynamic object extension])
])

