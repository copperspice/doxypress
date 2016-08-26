# project name, version, email
AC_INIT([DoxyPress], 1.2.4-RC1, [info@copperspice.com])

# location of temp autotools files
# note: space can not exist between ] and  )
AC_CONFIG_AUX_DIR([autotools/build-aux])
AC_CONFIG_MACRO_DIR([autotools/m4]) 

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# name of one source file
AC_CONFIG_SRCDIR([src/main.cpp])

# not a gnu project
AM_INIT_AUTOMAKE( [foreign] )

# checking build environment
LT_INIT
AC_PROG_INSTALL
AC_PROG_CC_C99
AC_PROG_CC_C_O
AC_PROG_CXX

# set flags for c+11
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([noext])

case "$target" in
*-*-mingw*)
   prefix=`pwd`/install   
   OSTYPE=WIN ;;
*-*-linux-*)
   OSTYPE=LINUX ;;
*-*-darwin*)
   OSTYPE=DARWIN ;;
esac

AM_CONDITIONAL(OSTYPE_WIN, test "$OSTYPE" == WIN)
AM_CONDITIONAL(OSTYPE_LINUX, test "$OSTYPE" == LINUX)
AM_CONDITIONAL(OSTYPE_DARWIN, test "$OSTYPE" == DARWIN)

echo
echo "Checking System headers"

AC_CHECK_HEADERS([windows.h], [HAVE_WINDOWS_H=yes])
AC_CHECK_FUNCS([fork getpid])

# display 32-bit or 64-bit
m4_pushdef([AS_MESSAGE_FD],[/dev/null]) 
AC_CHECK_SIZEOF(size_t)

case "$ac_cv_sizeof_size_t" in
4)
	BITSIZE="a 32" ;;
8)
	BITSIZE="a 64" ;;
*)
	BITSIZE="an unknown" ;;
esac
m4_popdef([AS_MESSAGE_FD]) 

echo
echo "Application configured for "$BITSIZE"-bit system" 
echo

# location of .am files that will create Makefiles
AC_CONFIG_FILES( [Makefile bin/Makefile   src/doxy_build_info.h])
AC_OUTPUT
