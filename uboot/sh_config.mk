#
# (C) Copyright 2004-2011
# Andy Sturges, STMicroelectronics, andy.sturges@st.com
# Sean McGoogan STMicroelectronics, <Sean.McGoogan@st.com>
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

# Update this for each release.
# The SPEC file will automatically substitute the date
# for actual build number, in the following identifier.
# Regular Expression for string is:
# 	"stm[2-9][0-9]-[2-9][0-9][0-9][0-9]-[01][0-9]-[0-3][0-9]"
#SH_IDENT_STRING="\"stm24_0056\""

PLATFORM_CPPFLAGS += -DCONFIG_SH4 -D__SH4__ -DCONFIG_IDENT_STRING="\"stm24_0056-$(BOARD)\""
PLATFORM_LDFLAGS  += -n
