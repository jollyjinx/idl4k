/*
 * (C) Copyright 2010 STMicroelectronics.
 *
 * Angus Clark   <Angus.Clark@st.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef STM_SPI_FSM_H
#define STM_SPI_FSM_H

	/*
	 * FSM SPI Controller Registers.
	 */
#define SPI_CLOCKDIV			0x0010
#define SPI_MODESELECT			0x0018
#define SPI_CONFIGDATA			0x0020
#define SPI_STA_MODE_CHANGE		0x0028
#define SPI_FAST_SEQ_TRANSFER_SIZE	0x0100
#define SPI_FAST_SEQ_ADD1		0x0104
#define SPI_FAST_SEQ_ADD2		0x0108
#define SPI_FAST_SEQ_ADD_CFG		0x010c
#define SPI_FAST_SEQ_OPC1		0x0110
#define SPI_FAST_SEQ_OPC2		0x0114
#define SPI_FAST_SEQ_OPC3		0x0118
#define SPI_FAST_SEQ_OPC4		0x011c
#define SPI_FAST_SEQ_OPC5		0x0120
#define SPI_MODE_BITS			0x0124
#define SPI_DUMMY_BITS			0x0128
#define SPI_FAST_SEQ_FLASH_STA_DATA	0x012c
#define SPI_FAST_SEQ_1			0x0130
#define SPI_FAST_SEQ_2			0x0134
#define SPI_FAST_SEQ_3			0x0138
#define SPI_FAST_SEQ_4			0x013c
#define SPI_FAST_SEQ_CFG		0x0140
#define SPI_FAST_SEQ_STA		0x0144
#define SPI_QUAD_BOOT_SEQ_INIT_1	0x0148
#define SPI_QUAD_BOOT_SEQ_INIT_2	0x014c
#define SPI_QUAD_BOOT_READ_SEQ_1	0x0150
#define SPI_QUAD_BOOT_READ_SEQ_2	0x0154
#define SPI_PROGRAM_ERASE_TIME		0x0158
#define SPI_MULT_PAGE_REPEAT_SEQ_1	0x015c
#define SPI_MULT_PAGE_REPEAT_SEQ_2	0x0160
#define SPI_STATUS_WR_TIME_REG		0x0164
#define SPI_FAST_SEQ_DATA_REG		0x0300

	/*
	 * Register: SPI_FAST_SEQ_TRANSFER_SIZE
	 */
#define TRANSFER_SIZE(x)		( (x) * 8 )	/* Transfer Size in BITS */

	/*
	 * Register: SPI_FAST_SEQ_ADD_CFG
	 */
#define ADR_CFG_CYCLES_ADD1(x)		((x) << 0)	/* number of cycles for address 1 */
#define ADR_CFG_PADS_1_ADD1		(0x0 << 6)	/* single I/O-mode */
#define ADR_CFG_PADS_2_ADD1		(0x1 << 6)	/* dual   I/O-mode */
#define ADR_CFG_PADS_4_ADD1		(0x3 << 6)	/* quad   I/O-mode */
#define ADR_CFG_CSDEASSERT_ADD1		(1   << 8)	/* de-assert CSn after address 1 */
#define ADR_CFG_CYCLES_ADD2(x)		((x) << (0+16))	/* number of cycles for address 2 */
#define ADR_CFG_PADS_1_ADD2		(0x0 << (6+16))	/* single I/O-mode */
#define ADR_CFG_PADS_2_ADD2		(0x1 << (6+16))	/* dual   I/O-mode */
#define ADR_CFG_PADS_4_ADD2		(0x3 << (6+16))	/* quad   I/O-mode */
#define ADR_CFG_CSDEASSERT_ADD2		(1   << (8+16))	/* de-assert CSn after address 2 */

	/*
	 * Register: SPI_FAST_SEQ_n
	 */
#define SEQ_OPC_OPCODE(x)		((x) << 0)	/* Flash Command */
#define SEQ_OPC_CYCLES(x)		((x) << 8)	/* number of cycles for command */
#define SEQ_OPC_PADS_1			(0x0 << 14)	/* single I/O-mode */
#define SEQ_OPC_PADS_2			(0x1 << 14)	/* dual   I/O-mode */
#define SEQ_OPC_PADS_4			(0x3 << 14)	/* quad   I/O-mode */
#define SEQ_OPC_CSDEASSERT		(1   << 16)	/* de-assert CSn after command */

	/*
	 * Register: SPI_FAST_SEQ_CFG
	 */
#define SEQ_CFG_STARTSEQ		(1 << 0)
#define SEQ_CFG_SWRESET			(1 << 5)	/* S/W reset */
#define SEQ_CFG_CSDEASSERT		(1 << 6)	/* de-assert CSn after QQQ (STOP/DATA) ??? */
#define SEQ_CFG_READNOTWRITE		(1 << 7)
#define SEQ_CFG_ERASE			(1 << 8)
#define SEQ_CFG_PADS_1			(0x0 << 16)	/* single I/O-mode */
#define SEQ_CFG_PADS_2			(0x1 << 16)	/* dual   I/O-mode */
#define SEQ_CFG_PADS_4			(0x3 << 16)	/* quad   I/O-mode */

	/*
	 * FSM SPI Instruction Opcodes.
	 */
#define FSM_OPC_CMD			0x1
#define FSM_OPC_ADD			0x2
#define FSM_OPC_STATUS_REG_DATA		0x3
#define FSM_OPC_MODE			0x4
#define FSM_OPC_DUMMY			0x5
#define FSM_OPC_DATA			0x6
#define FSM_OPC_WAIT			0x7
#define FSM_OPC_JUMP			0x8	/* QQQ - verify this number */
#define FSM_OPC_GOTO			0x9	/* QQQ - verify this number */
#define FSM_OPC_STOP			0xF

	/*
	 * FSM SPI Instructions (= opcode + operand).
	 */
#define FSM_INSTR(cmd, op)		((cmd) | ((op) << 4))

#define FSM_INST_CMD1			FSM_INSTR(FSM_OPC_CMD,	1)
#define FSM_INST_CMD2			FSM_INSTR(FSM_OPC_CMD,	2)
#define FSM_INST_CMD3			FSM_INSTR(FSM_OPC_CMD,	3)
#define FSM_INST_CMD4			FSM_INSTR(FSM_OPC_CMD,	4)
#define FSM_INST_CMD5			FSM_INSTR(FSM_OPC_CMD,	5)

#define FSM_INST_ADD1			FSM_INSTR(FSM_OPC_ADD,	1)
#define FSM_INST_ADD2			FSM_INSTR(FSM_OPC_ADD,	2)

#define FSM_INST_DATA_WRITE		FSM_INSTR(FSM_OPC_DATA,	1)
#define FSM_INST_DATA_READ		FSM_INSTR(FSM_OPC_DATA,	2)

#define FSM_INST_MODE			FSM_INSTR(FSM_OPC_MODE,	0)

#define FSM_INST_DUMMY			FSM_INSTR(FSM_OPC_DUMMY,0)

#define FSM_INST_WAIT			FSM_INSTR(FSM_OPC_WAIT,	0)

#define FSM_INST_STOP			FSM_INSTR(FSM_OPC_STOP,	0)


	/*
	 * Exported function declarations.
	 */
extern int fsm_init(void);
extern uint8_t fsm_read_status(void);
extern int fsm_read_jedec(
	const size_t bytes,
	uint8_t *const jedec);
extern int fsm_read(
	uint8_t * buf,
	const uint32_t size,
	uint32_t offset);
extern int fsm_erase_sector(
	const uint32_t offset,
	const uint8_t op_erase);
extern int fsm_write(
	const uint8_t *buf,
	const uint32_t bufsize,
	uint32_t offset);

	/* QQQ - move to spi.h ??? */
extern void spi_wait_till_ready(
	spi_chipsel_type const chipsel);

#endif	/* STM_SPI_FSM_H */


