#
# (C) Copyright 2008-2009 STMicroelectronics.
# Sean McGoogan <Sean.McGoogan@st.com>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#
# MB628 board:
#
#	Following are *Physical* Addresses:
#
#	Region		29-bit		32-bit		Size
#	------		------		------		----
#	LMI_BASE	0x0c000000	0x40000000	128MiB
#	VIDEO_BASE	0x0c000000	0x40000000	  4MiB
#	AUDIO_BASE	0x0c400000	0x40400000	  4MiB
#	eSTB_BASE	0x0c800000	0x40800000	 50MiB
#	eSTB_END	0x0f9fffff	0x439fffff	   ---
#
#
#	Following are *Virtual* Addresses:
#
#	Valid values for TEXT_BASE are:
#
#	0x8F900000	29-bit mode (Traditional Mode)
#	0x83900000	32-bit mode (Space-Enhancement Mode)
#
# Note:	Alternative definitions of TEXT_BASE are put into
#	'config.tmp' from the top-level 'Makefile'.
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef TEXT_BASE
# Installs at eSTB BASE + 49MB in P1 (cachable)
TEXT_BASE = 0x8F900000
endif

PLATFORM_LDFLAGS +=
