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

SUBDIRS = \
	flash \
	lib \
	tap \
	part \
	bus \
	cmd \
	global

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = urjtagmini.pc

lib_LTLIBRARIES = liburjtagmini.la
liburjtagmini_la_LDFLAGS = -version-info 0:0:0

liburjtagmini_la_SOURCES =

liburjtagmini_la_LIBADD = \
	tap/libtap.la \
	part/libpart.la \
	lib/libjtaglib.la \
	flash/libflash.la \
	bus/libbus.la \
	cmd/libcmd.la \
	global/libglobal.la \
	-lm \
	$(LIBFTDI_LIBS) \
	$(LIBUSB_LIBS)

AM_CFLAGS = $(WARNINGCFLAGS)
