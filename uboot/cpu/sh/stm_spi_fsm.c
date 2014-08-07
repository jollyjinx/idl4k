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

#include <common.h>
#include <asm/soc.h>
#include <asm/socregs.h>
#include <asm/io.h>
#include <spi.h>
#include <asm/clk.h>
#include <asm/spi-commands.h>
#include "stm_spi_fsm.h"


/**********************************************************************/


#if defined(CONFIG_STM_FSM_SPI)


/**********************************************************************/


#undef DEBUG
#if 0
#define DEBUG(args...)					\
	do {						\
		printf(args);				\
	} while (0)
#else
#define DEBUG(args...) do {} while (0)
#endif


/**********************************************************************/


#define ENABLE_ASSERTS

#if defined(ENABLE_ASSERTS)

static const char assert_message[] =
	"ERROR: Assertion failed in %s() @ %s:%u\n";

#define assert(x)					\
	do {						\
		if (!(x)) {				\
			printf(				\
				assert_message,		\
				__FUNCTION__,		\
				__FILE__,		\
				__LINE__);		\
		}					\
	} while (0)
#else	/* ENABLE_ASSERTS */

#define assert(x) do {} while (0)

#endif	/* ENABLE_ASSERTS */


/**********************************************************************/


static const unsigned long base_addr = CFG_STM_SPI_FSM_BASE;	/* SPI base */

#define fsm_write_reg(reg, val)		writel((val), base_addr + (reg))
#define fsm_read_reg(reg)		readl(base_addr + (reg))


#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})


#define FLASH_PAGESIZE		256	/* QQQ - Why not 512 ??? */
static uint8_t	page_buf[FLASH_PAGESIZE] __attribute__((aligned(4)));


/**********************************************************************/


struct fsm_seq {
	      uint32_t data_size;
	      uint32_t addr1;
	const uint32_t addr2;
	const uint32_t addr_cfg;
	const uint32_t seq_opc[5];
	const uint32_t mode;
	const uint32_t dummy;
	const uint32_t status;
	const uint8_t  seq[16];
	const uint32_t seq_cfg;
} __attribute__((__packed__,aligned(4)));

#define FSM_SEQ_SIZE		sizeof(struct fsm_seq)


/**********************************************************************/


static const struct fsm_seq seq_read_jedec = {
	.data_size = TRANSFER_SIZE(8),
	.seq_opc[0] = SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_READ_DEVID),
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_DATA_READ,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_READNOTWRITE |
		    SEQ_CFG_CSDEASSERT |
		    SEQ_CFG_STARTSEQ),
};

static const struct fsm_seq seq_read_status = {
	.data_size = TRANSFER_SIZE(4),
	.seq_opc[0] = SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_READ_STATUS),
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_DATA_READ,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_READNOTWRITE |
		    SEQ_CFG_CSDEASSERT |
		    SEQ_CFG_STARTSEQ),
};

static struct fsm_seq seq_read_data = {
	.seq_opc[0] = SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_READ_ARRAY),
	.addr_cfg = ADR_CFG_PADS_1_ADD1 | ADR_CFG_CYCLES_ADD1(24),
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_ADD1,
		FSM_INST_DATA_READ,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_READNOTWRITE |
		    SEQ_CFG_CSDEASSERT |
		    SEQ_CFG_STARTSEQ),
};

static struct fsm_seq seq_write_data = {
	.addr_cfg = ADR_CFG_PADS_1_ADD1 | ADR_CFG_CYCLES_ADD1(24),
	.seq_opc = {
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_WREN) | SEQ_OPC_CSDEASSERT),
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_PP)),
	},
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_CMD2,
		FSM_INST_ADD1,
		FSM_INST_DATA_WRITE,
		FSM_INST_WAIT,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_CSDEASSERT |
		    SEQ_CFG_STARTSEQ),
};

static struct fsm_seq seq_erase_sector = {
	.addr_cfg = ADR_CFG_PADS_1_ADD1 | ADR_CFG_CYCLES_ADD1(24) | ADR_CFG_CSDEASSERT_ADD1,
	.seq_opc = {
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_WREN) | SEQ_OPC_CSDEASSERT),
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_SE)),
	},
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_CMD2,
		FSM_INST_ADD1,
		FSM_INST_WAIT,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_ERASE |
		    SEQ_CFG_READNOTWRITE |
		    SEQ_CFG_CSDEASSERT |
		    0x2 |			/* QQQ - what is this for ??? */
		    SEQ_CFG_STARTSEQ),
};

#if 0
static const struct fsm_seq seq_erase_chip = {
	.seq_opc = {
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(OP_WREN) | SEQ_OPC_CSDEASSERT),
		(SEQ_OPC_PADS_1 | SEQ_OPC_CYCLES(8) | SEQ_OPC_OPCODE(FLASH_CMD_CHIPERASE)),
	},
	.seq = {
		FSM_INST_CMD1,
		FSM_INST_CMD2,
		FSM_INST_WAIT,
		FSM_INST_STOP,
	},
	.seq_cfg = (SEQ_CFG_PADS_1 |
		    SEQ_CFG_ERASE |
		    SEQ_CFG_READNOTWRITE |
		    SEQ_CFG_CSDEASSERT |
		    SEQ_CFG_STARTSEQ),
};
#endif


/**********************************************************************/


static inline int fsm_is_idle(void)
{
	return fsm_read_reg(SPI_FAST_SEQ_STA) & 0x10;
}

static inline int fsm_fifo_available(void)
{
	return fsm_read_reg(SPI_FAST_SEQ_STA) >> 5 & 0x7f;
}

static inline int fsm_set_mode(const uint32_t mode)
{
	while ((fsm_read_reg(SPI_STA_MODE_CHANGE) & 0x1) == 0);

	fsm_write_reg(SPI_MODESELECT, mode);

	return 0;
}

static void fsm_load_seq(const struct fsm_seq * const seq)
{
	uint32_t *a = (uint32_t *)((uint32_t)base_addr + SPI_FAST_SEQ_TRANSFER_SIZE);
	const uint32_t *s = (const uint32_t *)seq;
	int i = FSM_SEQ_SIZE/4;

		/* ensure the FSM is "idle" (not executing) */
	assert( fsm_is_idle() );

		/* program the sequence by writing to the FSM registers */
	while(i--) {
		writel(*s++, a++);
	}
}

static int fsm_wait_seq(void)
{
	int i = 0;

	while (!(fsm_is_idle()) && (i < 1000000)) {
		i++;
	}

	if (i == 1000000) {
		DEBUG("warning: in %s(), waited too long, reg=0x%08x\n",
			__func__,
			fsm_read_reg(SPI_FAST_SEQ_STA));
		mdelay(5);
	}

	return 0;
}

#if 0
static void fsm_clear_fifo(void)
{
	uint32_t avail;

	while ((avail = fsm_fifo_available()) > 0) {
		while (avail) {
			fsm_read_reg(SPI_FAST_SEQ_DATA_REG);
			avail--;
		}
	}
}
#endif

static int fsm_read_fifo(uint32_t *buf, const uint32_t size)
{
	uint32_t avail;
	uint32_t remaining = size/4;
	uint32_t words;
	uint32_t word;

		/* ensure parameters are 4-byte aligned */
	assert( (size & 0x3u) == 0u );
	assert( ((uint32_t)buf & 0x3u) == 0u );

	do {
		while (!(avail = fsm_fifo_available()))
			;	/* do nothing */

		words = min(avail, remaining);
		remaining -= words;
		avail -= words;
		while (words--)
		{
			word = fsm_read_reg(SPI_FAST_SEQ_DATA_REG);
			*buf++ = word;
		}
	} while (remaining);

	return (size - (remaining*4));
}

static int fsm_write_fifo(const uint32_t *buf, const uint32_t size)
{
	uint32_t remaining = size >> 2;

		/* ensure parameters are 4-byte aligned */
	assert( (size & 0x3u) == 0u );
	assert( ((uint32_t)buf & 0x3u) == 0u );

	while (remaining--) {
		fsm_write_reg(SPI_FAST_SEQ_DATA_REG, *buf);
		buf++;
	}

	return (size - (remaining*4));
}

extern uint8_t fsm_read_status(void)
{
	const struct fsm_seq * const seq = &seq_read_status;
	uint8_t status[4];

	fsm_load_seq(seq);
	fsm_read_fifo((uint32_t *)status, sizeof(status));

	/* Wait for FSM sequence to finish */
	fsm_wait_seq();

		/* return the LAST byte retrieved */
	return status[sizeof(status)-1u];
}

extern int fsm_read_jedec(const size_t bytes, uint8_t *const jedec)
{
	const struct fsm_seq * const seq = &seq_read_jedec;
	uint8_t tmp[8];		/* multiple of 4 */

	assert( bytes <= sizeof(tmp) );

	fsm_load_seq(seq);
	fsm_read_fifo((uint32_t *)tmp, sizeof(tmp));

	memcpy(jedec, tmp, bytes);

	return 0;
}

extern int fsm_erase_sector(const uint32_t offset, const uint8_t op_erase)
{
	struct fsm_seq * const seq = &seq_erase_sector;

	DEBUG("debug: in %s( offset=%p, op_erase=0x%02x )\n",
		__FUNCTION__, offset, op_erase);

	/* over-write the default erase op-code - this is dirty! */
	*(volatile uint8_t*)&seq->seq_opc[1] = op_erase;

	seq->addr1 = offset;
	fsm_load_seq(seq);

	/* Wait for FSM sequence to finish */
	fsm_wait_seq();

	/* Wait for SPI device to complete */
	spi_wait_till_ready(NULL);

	return 0;
}

#if 0
static int fsm_erase_chip(void)
{
	const struct fsm_seq * const seq = &seq_erase_chip;

	fsm_load_seq(seq);

	/* Wait for FSM sequence to finish */
	fsm_wait_seq();

	/* Wait for SPI device to complete */
	spi_wait_till_ready(NULL);

	return 0;
}
#endif

extern int fsm_read(uint8_t * buf, const uint32_t bufsize, uint32_t offset)
{
	struct fsm_seq * const seq = &seq_read_data;
	uint32_t size_ub;
	uint32_t size_lb;
	uint32_t size_mop;
	uint32_t tmp;
	uint8_t *p;
	uint32_t size, remaining=bufsize;

	/* Handle non-aligned buf */
	if ((uint32_t)buf & 0x3) {
		p = page_buf;	/* use a 4-byte aligned bounce buffer */
	} else {
		p = buf;	/* use the provided buffer */
	}

	do {
		/* no more than one FIFO at a time */
		size = min(remaining,sizeof(page_buf));

		DEBUG("debug: in %s() p=%p, size=0x%x, remaining=0x%x, offset=0x%x\n",
			__FUNCTION__, p, size, remaining, offset);

		/* Handle non-aligned size */
		size_ub = (size + 0x3) & ~0x3;
		size_lb = size & ~0x3;
		size_mop = size & 0x3;

		seq->data_size = TRANSFER_SIZE(size_ub);	/* in bits! */
		seq->addr1 = offset;

		fsm_load_seq(seq);

		fsm_read_fifo((uint32_t *)p, size_lb);

		if (size_mop) {
			fsm_read_fifo(&tmp, sizeof(tmp));
			memcpy(p + size_lb, &tmp, size_mop);
		}

		/* Handle non-aligned buf */
		if (page_buf == p) {	/* used bounce buffer ? */
			memcpy(buf, p, size);
			buf+=size;
		} else {		/* used provided buffer */
			p += size;
		}

		/* prepare for next iteration */
		offset += size;
		remaining -= size;

		/* Wait for FSM sequence to finish */
		fsm_wait_seq();

	} while (remaining);

	return 0;
}

extern int fsm_write(const uint8_t * const buf, const uint32_t bufsize, uint32_t offset)
{
	struct fsm_seq * const seq = &seq_write_data;
	uint32_t size_ub;
	uint32_t size_lb;
	uint32_t size_mop;
	int i;
	const uint8_t *p=buf;
	uint32_t size, remaining=bufsize;
	uint32_t tmp;
	uint8_t *t = (uint8_t *)&tmp;

		/* ensure the FSM is "idle" (not executing) */
	assert( fsm_is_idle() );

	do {
		/* no more than one FIFO at a time */
		size = min(remaining,sizeof(page_buf));

		DEBUG("debug: in %s() p=%p, size=0x%x, remaining=0x%x, offset=0x%x\n",
			__FUNCTION__, p, size, remaining, offset);

		/* Handle non-aligned buf */
		if ((uint32_t)buf & 0x3) {
			p = page_buf;	/* use a 4-byte aligned bounce buffer */
			memcpy(page_buf, buf, size);
		} else {
					/* use the provided buffer */
		}

		/* Handle non-aligned size */
		size_ub = (size + 0x3) & ~0x3;
		size_lb = size & ~0x3;
		size_mop = size & 0x3;

		seq->data_size = TRANSFER_SIZE(size_ub);	/* in bits! */
		seq->addr1 = offset;

		/* prepare to write the data to the FIFO in the FSM */
		/* ensure at least SPI_FAST_SEQ_CFG[0,5,7] are all zero */
		fsm_write_reg(SPI_FAST_SEQ_CFG, 0x0);
		/* force the write to complete before proceeding! */
		(void)fsm_read_reg(SPI_FAST_SEQ_CFG);

		/* now, write the data to the FIFO in the FSM */
		fsm_write_fifo((uint32_t *)p, size_lb);
		p += size_lb;
		/* Handle non-aligned size */
		if (size_mop) {
			tmp = ~0ul;	/* fill with 0xFF's */
			for (i = 0; i < size_mop; i++)
				t[i] = *p++;
			fsm_write_fifo(&tmp, sizeof(tmp));
		}

		/* Start sequence */
		fsm_load_seq(seq);

		/* prepare for next iteration */
		offset += size;
		remaining -= size;

		/* Wait for FSM sequence to finish */
		fsm_wait_seq();

		/* Wait for SPI device to complete */
		spi_wait_till_ready(NULL);

	} while (remaining);

	return 0;
}


#if !defined(CFG_STM_SPI_CLOCKDIV)
#define CFG_STM_SPI_CLOCKDIV		2	/* default is SPI_CLOCKDIV = 2 */
#endif

extern int fsm_init(void)
{
	DEBUG("debug: in %s()\n", __FUNCTION__);

		/* perform a soft reset of the FSM controller */
	fsm_write_reg(SPI_FAST_SEQ_CFG, SEQ_CFG_SWRESET);
	udelay(100);	/* QQQ - is this long enough ??? */
	fsm_write_reg(SPI_FAST_SEQ_CFG, 0);

	fsm_write_reg(SPI_CLOCKDIV, CFG_STM_SPI_CLOCKDIV);

	/* select Fast Sequence Mode (FSM) */
	fsm_set_mode(1<<3);

	fsm_write_reg(SPI_CONFIGDATA, CFG_STM_SPI_CONFIGDATA);
	fsm_write_reg(SPI_PROGRAM_ERASE_TIME, CFG_STM_SPI_PROGRAM_ERASE_TIME);

	assert( fsm_fifo_available() == 0 );
/*	fsm_clear_fifo();	*/

	return 0;
}


/**********************************************************************/


#endif	/* defined(CONFIG_STM_FSM_SPI) */


