#
# $Id$
#
# Copyright (C) 2007, 2008 Kolja Waschk and other
#    UrJTAG.org developers, (C) 2002, 2003 ETC s.r.o.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# Written by Marcel Telka <marcel@telka.sk>, 2002, 2003,
#  and other UrJTAG authors, 2007, 2008, ...
#

AC_INIT([UrJTAG-mini], [0.10], [], [urjtag-mini])

dnl autoconf-2.64 was released in 2008
AC_PREREQ(2.64)
AC_COPYRIGHT([Copyright (C) 2007/08 Kolja Waschk and other UrJTAG authors; 2002/03 ETC s.r.o.])
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(tools)

dnl automake-1.10 was released in 2006
AM_INIT_AUTOMAKE([1.10 check-news dist-bzip2])
dnl silent rules requires automake-1.11+
m4_ifdef([AM_SILENT_RULES],
	[AM_SILENT_RULES([yes])],
	[AC_SUBST([AM_DEFAULT_VERBOSITY], [1])])

AC_CONFIG_FILES(
	Makefile
	data/Makefile
	include/Makefile
	include/urjtag/Makefile
	src/Makefile
	src/urjtagmini.pc
	src/lib/Makefile
	src/tap/Makefile
	src/part/Makefile
	src/bus/Makefile
	src/flash/Makefile
	src/cmd/Makefile
	src/global/Makefile
	src/apps/jtag/Makefile
)

AM_MAINTAINER_MODE

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_HEADERS(include/urjtag/urjtag.h)

AC_GNU_SOURCE

LT_INIT
AC_PROG_CC
AC_PROG_YACC
AC_PROG_CC_C99
PKG_PROG_PKG_CONFIG

AC_CHECK_FUNCS(m4_flatten([
	_sleep
	getdelim
	geteuid
	getline
	getuid
	nanosleep
	pread
	swprintf
	usleep
]))

AC_CHECK_FUNC(clock_gettime, [], [ AC_CHECK_LIB(rt, clock_gettime) ])


dnl check for sigaction with SA_ONESHOT or SA_RESETHAND
AC_TRY_COMPILE([#include <signal.h>], [
#ifndef SA_ONESHOT
#  define SA_ONESHOT SA_RESETHAND
#endif
int sa_flags = SA_ONESHOT;
],
[AC_DEFINE([HAVE_SIGACTION_SA_ONESHOT], [1], [Have a defined SA_ONESHOT])],
[])

AC_CHECK_HEADERS([linux/ppdev.h], [HAVE_LINUX_PPDEV_H="yes"])
AC_CHECK_HEADERS(m4_flatten([
	wchar.h
	windows.h
	sys/wait.h
]))

AC_CHECK_TYPE([wchar_t], [],
	[AC_DEFINE([wchar_t], [char], [Make sure we have wchar_t])],
	[#include <stddef.h>])

AC_CHECK_TYPE([struct timespec],
	[AC_DEFINE(HAVE_STRUCT_TIMESPEC, 1, [Define to 1 if you have the struct timespec type])],
	[], [[#include <time.h>]])


VL_LIB_READLINE


dnl check for libusb

AC_ARG_WITH([libusb],
  [AS_HELP_STRING([--with-libusb], [lib for USB JTAG cables @<:@default=detect 1.0 0.1@:>@])],,
    [with_libusb=detect])

HAVELIBUSB=no
LIBUSB_LIBS=
AS_IF([test "x$with_libusb" != xno], [
  AS_IF([test "x$with_libusb" != x0.1 -a \
              "x$with_libusb" != x1.0 -a \
              "x$with_libusb" != xdetect -a \
              "x$with_libusb" != xyes], [
    HAVELIBUSB=manual
    LIBUSB_LIBS="-L$with_libusb -L$with_libusb/lib -lusb"
    LIBUSB_CFLAGS="-I$with_libusb -I$with_libusb/include"
  ], [
    AS_IF([test "x$with_libusb" != x0.1 -a "x$HAVELIBUSB" = xno], [
      PKG_CHECK_MODULES(LIBUSB, libusb-1.0, [
          HAVELIBUSB=1.0
          AC_DEFINE(HAVE_LIBUSB1, 1, [Define if you have libusb-1.0])
        ], [:])
    ])
    AS_IF([test "x$with_libusb" != x1.0 -a "x$HAVELIBUSB" = xno], [
      PKG_CHECK_MODULES(LIBUSB, libusb, [HAVELIBUSB=0.1], [:])
    ])
    AS_IF([test "x$HAVELIBUSB" = xno], [
      AS_IF([test "x$with_libusb" != xdetect], [
        AC_MSG_ERROR([*** libusb not detected])
      ], [
        AC_MSG_WARN([*** libusb not detected. No support for USB JTAG cables via libusb.])
      ])
    ],[
      AC_DEFINE(HAVE_LIBUSB, 1, [Define if you have libusb])
    ])
  ])
  AS_VAR_APPEND([CPPFLAGS], " $LIBUSB_CFLAGS")
])
AC_SUBST(LIBUSB_LIBS)
AM_CONDITIONAL(HAVE_LIBUSB, [test "x$HAVELIBUSB" != "xno"])


dnl Use FTDI library?

AC_ARG_WITH([libftdi],
  [AS_HELP_STRING([--with-libftdi],
    [use libftdi for FTDI-based cables])],
    [], [with_libftdi=check])

HAVELIBFTDI=no
LIBFTDI_LIBS=
AS_IF([test "x$with_libftdi" != xno], [
  save_LIBS=$LIBS
  save_CPPFLAGS=$CPPFLAGS
  PKG_CHECK_MODULES(LIBFTDI, libftdi1, [:;], [dnl
    PKG_CHECK_MODULES(LIBFTDI, libftdi, [:;], [dnl
      AS_IF([test "x$with_libftdi" != xyes -a "x$with_libftdi" != xcheck], [
        LIBFTDI_LIBS="-L$with_libftdi -lftdi"
        LIBFTDI_CFLAGS="-I$with_libftdi"
      ],[
        AC_PATH_TOOL(LIBFTDI_CONFIG, libftdi-config, no)
        AS_IF([test "$LIBFTDI_CONFIG" != "no"],[
          LIBFTDI_LIBS=`$LIBFTDI_CONFIG --libs`
          LIBFTDI_CFLAGS=`$LIBFTDI_CONFIG --cflags`
        ])
      ])
    ])
  ])
  AS_VAR_APPEND([LIBS], " $LIBFTDI_LIBS")
  AS_VAR_APPEND([CPPFLAGS], " $LIBFTDI_CFLAGS")
  AC_CHECK_FUNC([ftdi_usb_open], [
    AC_DEFINE(HAVE_LIBFTDI, 1, [Define if you have libftdi])
    HAVELIBFTDI=yes
  ],[
    AC_MSG_WARN([*** libftdi not detected. No support for FTDI-based USB JTAG cables via libftdi.])
    CPPFLAGS=$save_CPPFLAGS
  ])
  AC_CHECK_FUNC([ftdi_read_data_submit], [
    AS_IF([test "x$HAVELIBUSB" = "x0.1"], [
      AC_MSG_ERROR([this libftdi cannot be used with libusb-0.1, libusb-1.0 is needed])
    ])
    AC_DEFINE(HAVE_LIBFTDI_ASYNC_MODE, 1, [Define if libftdi support async mode])
    HAVELIBFTDI_ASYNCMODE=yes
  ])
  LIBS=$save_LIBS
])
AC_SUBST(LIBFTDI_LIBS)
AM_CONDITIONAL(HAVE_LIBFTDI, [test "x$HAVELIBFTDI" = "xyes"])
AM_CONDITIONAL(HAVE_LIBFTDI_ASYNC_MODE, [test "x$HAVELIBFTDI_ASYNCMODE" = "xyes"])


WARNINGCFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith"
AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--enable-werror], [build with -Werror])],
  [], [enable_werror=maybe])
AS_IF([test "x$enable_werror" = xmaybe -a -d .svn], [
  enable_werror=yes
])
AS_IF([test "x$enable_werror" = xyes], [
  WARNINGCFLAGS="$WARNINGCFLAGS -Werror"
])
AC_SUBST(WARNINGCFLAGS)

AS_VAR_PREPEND([CPPFLAGS], '-I$(top_srcdir) -I$(top_srcdir)/include ')

# check for lex/flex
AC_PROG_LEX
# check for modern version of flex
AC_PROG_AWK
AC_PROG_SED

dnl Enable experimental brute-force JEDEC flash autodetection?
AC_ARG_ENABLE(jedec-exp,
[AS_HELP_STRING([--enable-jedec-exp], [Enable experimental JEDEC flash detection])],
[case "${enableval}" in
   yes) jedecexp=true ;;
   no)  jedecexp=false ;;
   *)   AC_MSG_ERROR(bad value ${enableval} for --enable-jedec-exp) ;;
 esac],
[jedecexp=false])
AS_IF([test "x$jedecexp" = xtrue], [
  AM_CONDITIONAL(JEDEC_EXP, true)
  AC_DEFINE(JEDEC_EXP, 1, [define if experimental JEDEC flash detection is enabled])
],[
  AM_CONDITIONAL(JEDEC_EXP, false)
])


dnl
dnl URJ_DRIVER_SET([driver set name],
dnl                [drivers to enable by default],
dnl                [drivers to disable by default],
dnl                [extra code to run before processing user list])
dnl
dnl Only the first two arguments are required.  The extra code is useful to
dnl automatically strip out drivers if required external libraries were not
dnl found.
dnl
AC_DEFUN([URJ_DRIVER_SET], [
	AC_DEFUN([DEF_ENABLE_DRIVERS], m4_flatten([$2]))
	AC_DEFUN([DEF_DISABLE_DRIVERS], m4_flatten([$3]))
	AC_DEFUN([ALL_DRIVERS], DEF_ENABLE_DRIVERS DEF_DISABLE_DRIVERS)

AC_ARG_ENABLE($1,
[AS_HELP_STRING([--enable-]$1, [Enable default set or specific ]$1[ drivers:])]
[AS_HELP_STRING([],            ['default' enables:])]
[AS_HELP_STRING([],DEF_ENABLE_DRIVERS)]
m4_ifnblank([$3], [
[AS_HELP_STRING([],            [Disabled by default are:])]
[AS_HELP_STRING([],DEF_DISABLE_DRIVERS)]
])dnl
,
[AS_CASE([${enableval}],
    [yes],  [drivers=default],
    [no],   [drivers=none],
    [none], [drivers=none],
            [drivers=`echo ${enableval} | $SED -e 's/,/ /g'`])]
,
[drivers=default])
	# expand 'default' to default enabled $1 drivers
	drivers=`echo ${drivers} | $SED -e "s/default/DEF_ENABLE_DRIVERS/"`
	m4_ifnblank([$4], $4)

	enabled_drivers=''
	m4_foreach_w([x], ALL_DRIVERS, [
		AC_DEFUN([DRIVER_DEFINE], m4_toupper([ENABLE_]$1[_]x))
		AS_IF([echo "$drivers" | $GREP -q "]x["], [
			AC_DEFINE(DRIVER_DEFINE, 1, [define if ]x[ is enabled])
			AM_CONDITIONAL(DRIVER_DEFINE, true)
			AS_VAR_APPEND([enabled_drivers], "x ")
		], [
			AM_CONDITIONAL(DRIVER_DEFINE, false)
		])
	])
	[enabled_]$1[_drivers]=$enabled_drivers
	$1[drivers]=$drivers
])



# Enable bus drivers
URJ_DRIVER_SET([bus], [
	arm9tdmi
	pxa2x0
	pxa27x
	s3c4510
	sa1110
])


# Enable cable drivers
URJ_DRIVER_SET([cable], [
	dlc5
	ft2232
	wiggler
],[
],[
	# automatically disable cable drivers when a required feature is not available
	AS_IF([test "x$HAVELIBFTDI" != "xyes" -a "x$HAVELIBFTD2XX" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ft2232//" -e "s/usbblaster//"`
	])
])

# Enable lowlevel drivers
URJ_DRIVER_SET([lowlevel], [
	ftdi
	ppdev
],[
],[
	# automatically disable lowlevel drivers when a required feature is not available
	AS_IF([test "x$HAVELIBFTDI" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ftdi//"`
	])
	AS_IF([test "x$HAVE_LINUX_PPDEV_H" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ppdev//"`
	])
])


dnl Enable a relocatable jtag?
AC_ARG_ENABLE(relocatable,
[AS_HELP_STRING([--enable-relocatable], [Enable relocatable paths])],
[relocatable=$enableval], [relocatable=no])
AS_IF([test "x$relocatable" = xyes], [
  AM_CONDITIONAL(JTAG_RELOCATABLE, true)
  AC_DEFINE(JTAG_RELOCATABLE, 1, [define for relocatable paths])
],[
  AM_CONDITIONAL(JTAG_RELOCATABLE, false)
])


dnl Enable use of DMALLOC library?
AC_ARG_ENABLE(dmalloc,
[AS_HELP_STRING([--enable-dmalloc], [Enable heap debugging with dmalloc library])],
[dmalloc=$enableval], [dmalloc=no])
AS_IF([test "x$dmalloc" = xyes], [
  AM_CONDITIONAL(DMALLOC, true)
  AC_DEFINE(DMALLOC, 1, [define for dmalloc library])
],[
  AM_CONDITIONAL(DMALLOC, false)
])


# Enable flash multi-byte write mode?
AC_ARG_ENABLE(flash-multi-byte,
[AS_HELP_STRING([--disable-flash-multi-byte], [Disable flash multi-byte write mode])],
[flash_multi_byte=$enableval], [flash_multi_byte=yes])
AS_IF([test "x$flash_multi_byte" = xyes], [
  AC_DEFINE(FLASH_MULTI_BYTE, 1, [define for flash multi-byte write mode])
])


AC_OUTPUT

dnl
dnl Configuration summary
dnl
FLAG_HAVELIBUSB=$HAVELIBUSB
AS_IF([test "x$HAVELIBFTDI_ASYNCMODE" = "xyes"], [
  FLAG_HAVELIBFTDI_ASYNCMODE="(have async mode)"
],[
  FLAG_HAVELIBFTDI_ASYNCMODE="(no async mode)"
])
AC_DEFUN([MAKE_YESNO_VAR],[dnl
  AS_IF([test ${$1:-$2} != $2], [dnl
     FLAG_$1=yes
  ], [dnl
     FLAG_$1=no
  ])
])
MAKE_YESNO_VAR([HAVELIBFTDI], [no])
AC_MSG_NOTICE([

urjtag-mini is now configured for

  Libraries:
    libusb     : $FLAG_HAVELIBUSB
    libftdi    : $FLAG_HAVELIBFTDI $FLAG_HAVELIBFTDI_ASYNCMODE

  Drivers:
    Bus        : $enabled_bus_drivers
    Cable      : $enabled_cable_drivers
    Lowlevel   : $enabled_lowlevel_drivers
])
