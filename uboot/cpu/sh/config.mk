#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
#
# (C) Copyright 2009 STMicroelectronics.
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

### old:  PLATFORM_CPPFLAGS += -m4 -m4-nofpu -U__sh3__
### want: PLATFORM_CPPFLAGS += -m4 -mno-implicit-fp
PLATFORM_CPPFLAGS += -m4 -U__sh3__ -D__SH4__=1

ifeq ($(UBOOT_DEBUG), 1)
PLATFORM_CPPFLAGS += -O0
endif

# all SH (ST40) CPUs will use the same linker script file
ifeq ("$(LDSCRIPT)", "")
LDSCRIPT := $(SRCTREE)/cpu/$(CPU)/u-boot.lds
endif

