#
# $Id$
#
# Copyright (C) 2003 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2003.
#

include $(top_srcdir)/Makefile.rules

noinst_LTLIBRARIES = libbus.la

libbus_la_SOURCES = \
	buses.c \
	buses.h \
	buses_list.h \
	generic_bus.c \
	generic_bus.h \
	pxa2x0_mc.h \
	readmem.c \
	writemem.c

if ENABLE_BUS_ARM9TDMI
libbus_la_SOURCES += arm9tdmi.c
endif

if ENABLE_BUS_PXA2X0
libbus_la_SOURCES += pxa2x0.c
else
if ENABLE_BUS_PXA27X
libbus_la_SOURCES += pxa2x0.c
endif
endif

if ENABLE_BUS_S3C4510
libbus_la_SOURCES += s3c4510x.c
endif

if ENABLE_BUS_SA1110
libbus_la_SOURCES += sa1110.c
endif

AM_CFLAGS = $(WARNINGCFLAGS)
