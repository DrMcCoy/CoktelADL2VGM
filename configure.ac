AC_PREREQ([2.72])

AC_INIT([CoktelADL2VGM],[1.0.2],[drmccoy@drmccoy.de],[cokteladl2vgm],[https://github.com/DrMcCoy/CoktelADL2VGM])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.11 -Wall -Wno-portability no-dist-gzip dist-xz subdir-objects])
# Support silent build rules. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
AM_SILENT_RULES([yes])

dnl If AM_PROG_AR exists, call it, to shut up a libtool warning (and make it more portable, I guess?)
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl libtool
LT_PREREQ([2.2])
LT_INIT([disable-shared])

dnl We're C++
AC_PROG_CXX
AM_PROG_CC_C_O
dnl We want a make install
AC_PROG_INSTALL
dnl Progs considered necessary by autoscan
AC_PROG_AR
AC_PROG_CPP
AC_PROG_RANLIB

dnl --with-werror
AC_ARG_WITH([werror], [AS_HELP_STRING([--with-werror], [Compile with -Werror @<:@default=no@:>@])], [], [with_werror=no])
if test "x$with_werror" = "xyes"; then
	WERROR="-Werror -Werror=unused-but-set-variable"
fi

dnl Standard C, C++
AC_C_CONST

dnl Endianness
AC_C_BIGENDIAN()

dnl Inline considered necessary by autoscan
AC_C_INLINE

dnl Special variables of the size of pointers
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T
dnl Type check considered necessary by autoscan
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

dnl Extra checks considered necessary by autoscan
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_FUNC_ERROR_AT_LINE

dnl Extra flags
case "$target" in
	*darwin*)
		ADL2VGM_CFLAGS="-DUNIX -DMACOSX"
		ADL2VGM_LIBS=""
		;;
	*mingw*)
		ADL2VGM_CFLAGS=""
		ADL2VGM_LIBS="-static-libgcc -static-libstdc++"
		;;
	*)
		ADL2VGM_CFLAGS="-DUNIX"
		ADL2VGM_LIBS=""
		;;
esac;

AC_SUBST(ADL2VGM_CFLAGS)
AC_SUBST(ADL2VGM_LIBS)

AC_SUBST(WERROR)

AC_CONFIG_FILES([src/common/Makefile])
AC_CONFIG_FILES([src/adlib/Makefile])
AC_CONFIG_FILES([src/gob/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
