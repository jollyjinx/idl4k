#
# (C) Copyright 2003-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

include $(TOPDIR)/config.mk

LIB	= libfta.a

COBJS-y += fta_main.o

COBJS-y += display/display.o

COBJS-y += graphics/text.o
COBJS-y += graphics/graphics.o

COBJS-y += cmd/cmd_fta.o
COBJS-y += cmd/cmd_pls.o

COBJS-y += iboot/load_module.o
COBJS-y += iboot/utilib.o
COBJS-y += iboot/nand.o
COBJS-y += iboot/ssa.o
COBJS-y += zlib/inflate.o

COBJS-y += fp/fp.o

SOBJS	:= $(SOBJS-y)
COBJS	:= $(COBJS-y)

################################
OBJS	= $(COBJS) $(SOBJS)

$(LIB):	.depend $(OBJS)
	$(AR) crv $@ $(OBJS)
##################################
.depend:	Makefile $(SOBJS:.o=.S) $(COBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(COBJS:.o=.c) > $@
sinclude .depend
##################################
