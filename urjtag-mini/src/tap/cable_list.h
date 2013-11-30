/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */


/*
 * Please keep this list sorted alphabetically
 */

#ifdef ENABLE_CABLE_DLC5
_URJ_CABLE(dlc5)
#endif
#ifdef ENABLE_CABLE_FT2232
_URJ_CABLE(ft2232)
_URJ_CABLE(ft2232_armusbocd)
_URJ_CABLE(ft2232_armusbtiny_h)
_URJ_CABLE(ft2232_flyswatter)
_URJ_CABLE(ft2232_gnice)
_URJ_CABLE(ft2232_gniceplus)
_URJ_CABLE(ft2232_jtagkey)
_URJ_CABLE(ft2232_ktlink)
_URJ_CABLE(ft2232_milkymist)
_URJ_CABLE(ft2232_oocdlinks)
_URJ_CABLE(ft2232_signalyzer)
_URJ_CABLE(ft2232_turtelizer2)
_URJ_CABLE(ft2232_usbjtagrs232)
_URJ_CABLE(ft2232_usbscarab2)
_URJ_CABLE(ft2232_usbtojtagif)
#endif
#ifdef ENABLE_CABLE_WIGGLER
_URJ_CABLE(minimal)
#endif
#ifdef ENABLE_CABLE_WIGGLER
_URJ_CABLE(wiggler)
_URJ_CABLE(wiggler2)
#endif

#undef _URJ_CABLE
