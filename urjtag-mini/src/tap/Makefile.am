#
# $Id$
#
# Copyright (C) 2002 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2002.
#

include $(top_srcdir)/Makefile.rules

noinst_LTLIBRARIES = libtap.la

libtap_la_SOURCES = \
	tap.c \
	register.c \
	state.c \
	chain.c \
	detect.c \
	discovery.c \
	idcode.c \
	parport.c \
	parport.h \
	parport_list.h \
	usbconn.c \
	usbconn.h \
	usbconn_list.h \
	cable.c \
	cable.h \
	cable_list.h \
	cable/generic.h \
	cable/generic.c \
	cable/generic_usbconn.h \
	cable/generic_usbconn.c \
	cable/generic_usbconn_list.h \
	cable/generic_parport.h \
	cable/generic_parport.c \
	cable/cmd_xfer.h \
	cable/cmd_xfer.c

if ENABLE_CABLE_DLC5
libtap_la_SOURCES += \
	cable/dlc5.c
endif

if ENABLE_CABLE_WIGGLER
libtap_la_SOURCES += \
	cable/minimal.c \
	cable/wiggler.c \
	cable/wiggler2.c
endif

if ENABLE_CABLE_FT2232
libtap_la_SOURCES += \
	cable/ft2232.c
endif

if HAVE_LIBUSB
libtap_la_SOURCES += \
	usbconn/libusb.c \
	usbconn/libusb.h
endif

if ENABLE_LOWLEVEL_FTDI
libtap_la_SOURCES += \
	usbconn/libftdi.c \
	usbconn/libftdx.h
endif

if ENABLE_LOWLEVEL_PPDEV
libtap_la_SOURCES += \
	parport/ppdev.c
endif

AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = $(WARNINGCFLAGS)
