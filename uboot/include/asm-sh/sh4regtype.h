/*
 * Copyright (C) STMicroelectronics Ltd. 2002.
 *
 * andy.sturges@st.com
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

#ifndef __SH4REGTYPE_H
#define __SH4REGTYPE_H


#ifndef __ASSEMBLY__

typedef volatile unsigned char *const		sh4_byte_reg_t;
typedef volatile unsigned short *const		sh4_word_reg_t;
typedef volatile unsigned int *const		sh4_dword_reg_t;
typedef volatile unsigned long long *const	sh4_gword_reg_t;

#define SH4_BYTE_REG(address)	((sh4_byte_reg_t) (address))
#define SH4_WORD_REG(address)	((sh4_word_reg_t) (address))
#define SH4_DWORD_REG(address)	((sh4_dword_reg_t) (address))
#define SH4_GWORD_REG(address)	((sh4_gword_reg_t) (address))

#else	/* __ASSEMBLY__ */

#define SH4_BYTE_REG(address)	(address)
#define SH4_WORD_REG(address)	(address)
#define SH4_DWORD_REG(address)	(address)
#define SH4_GWORD_REG(address)	(address)

#endif	/* __ASSEMBLY__ */


#endif /* __SH4REGTYPE_H */
