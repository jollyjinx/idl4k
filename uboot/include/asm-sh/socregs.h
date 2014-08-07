/*
 * Copyright (C) STMicroelectronics Ltd. 2004-2010
 *
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef __SOC_REG_H
#define __SOC_REG_H


#if defined(CONFIG_SH_STB7100)
#	include <asm/stb7100reg.h>
#elif defined(CONFIG_SH_STX5197)
#	include <asm/stx5197reg.h>
#elif defined(CONFIG_SH_STX5206)
#	include <asm/stx5206reg.h>
#elif defined(CONFIG_SH_STX7105)
#	include <asm/stx7105reg.h>
#elif defined(CONFIG_SH_STX7108)
#	include <asm/stx7108reg.h>
#elif defined(CONFIG_SH_STX7111)
#	include <asm/stx7111reg.h>
#elif defined(CONFIG_SH_STX7141)
#	include <asm/stx7141reg.h>
#elif defined(CONFIG_SH_STX7200)
#	include <asm/stx7200reg.h>
#elif defined(CONFIG_SH_FLI7510)
#	include <asm/fli7510reg.h>
#elif defined(CONFIG_SH_FLI7540)
#	include <asm/fli7540reg.h>
#else
#	error Missing Device Definitions!
#endif


#endif /* __SOC_REG_H */
