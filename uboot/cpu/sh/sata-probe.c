/*
 * STM SATA initialization
 *
 * Copyright (C) 2005-2010 STMicroelectronics Limited
 * Stuart Menefy <stuart.menefy@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * This file is based on "drivers/ata/sata_stm.c"
 * in STMicroelectronics' release "stm23_0119" of the linux kernel.
 */

#include <common.h>


#if defined(CONFIG_SH_STM_SATA)


#include <asm/errno.h>
#include <asm/st40reg.h>
#include <asm/io.h>
#include <asm/soc.h>
#include <asm/socregs.h>


#if defined(CONFIG_SH_STX7105)		/* Cut 3.x (or later) */
#define pc_glue_logic_init	0	/* arch/sh/kernel/cpu/sh4/setup-stx7105.c */
#elif defined(CONFIG_SH_STX7141)	/* Cut 2.x (or later) */
#define pc_glue_logic_init	0	/* arch/sh/kernel/cpu/sh4/setup-stx7141.c */
#elif defined(CONFIG_SH_STX7200)	/* Cut 3.x (or later) */
#define pc_glue_logic_init	0	/* arch/sh/kernel/cpu/sh4/setup-stx7200.c */
#else
#	error Missing Device Definitions!
#endif


/*
 * Bases addresses of the SATA component blocks:
 *	Wrapper registers		0x000 - 0x3FF
 *	DMA Controller registers	0x400 - 0x7FF
 *	SATA Host Controller		0x800 - 0xBFF
 */
#define SATA_BASE_ADDR				(CFG_ATA_BASE_ADDR & ~0xfff)	/* 4K aligned */
#define SATA_AHB2STBUS_BASE			(SATA_BASE_ADDR + 0x000)
#define SATA_AHBDMA_BASE			(SATA_BASE_ADDR + 0x400)
#define SATA_AHBHOST_BASE			(SATA_BASE_ADDR + 0x800)

/* AHB_STBus protocol converter */
#define SATA_AHB2STBUS_STBUS_OPC		(SATA_AHB2STBUS_BASE + 0x000)
#define SATA_AHB2STBUS_MESSAGE_SIZE_CONFIG	(SATA_AHB2STBUS_BASE + 0x004)
#define SATA_AHB2STBUS_CHUNK_SIZE_CONFIG	(SATA_AHB2STBUS_BASE + 0x008)
#define SATA_AHB2STBUS_SW_RESET			(SATA_AHB2STBUS_BASE + 0x00c)
#define SATA_AHB2STBUS_PC_STATUS		(SATA_AHB2STBUS_BASE + 0x010)
#define SATA_PC_GLUE_LOGIC			(SATA_AHB2STBUS_BASE + 0x014)
#define SATA_PC_GLUE_LOGICH			(SATA_AHB2STBUS_BASE + 0x018)

/* AHB host controller */
#define SATA_CDR0				(SATA_AHBHOST_BASE + 0x000)
#define SATA_CDR1				(SATA_AHBHOST_BASE + 0x004)
#define SATA_CDR2				(SATA_AHBHOST_BASE + 0x008)
#define SATA_CDR3				(SATA_AHBHOST_BASE + 0x00c)
#define SATA_CDR4				(SATA_AHBHOST_BASE + 0x010)
#define SATA_CDR5				(SATA_AHBHOST_BASE + 0x014)
#define SATA_CDR6				(SATA_AHBHOST_BASE + 0x018)
#define SATA_CDR7				(SATA_AHBHOST_BASE + 0x01c)
#define SATA_CLR0				(SATA_AHBHOST_BASE + 0x020)
#define SATA_SCR0				(SATA_AHBHOST_BASE + 0x024)
#define SATA_SCR1				(SATA_AHBHOST_BASE + 0x028)
#define SATA_SCR2				(SATA_AHBHOST_BASE + 0x02c)
#define SATA_SCR3				(SATA_AHBHOST_BASE + 0x030)
#define SATA_SCR4				(SATA_AHBHOST_BASE + 0x034)
#define SATA_DMACR				(SATA_AHBHOST_BASE + 0x070)
#define SATA_DBTSR				(SATA_AHBHOST_BASE + 0x074)
#define SATA_PHYCR				(SATA_AHBHOST_BASE + 0x088)
#define SATA_VERSIONR				(SATA_AHBHOST_BASE + 0x0f8)
#define SATA_INTPR				(SATA_AHBHOST_BASE + 0x078)

/* AHB DMA controller */
#define DMAC_COMP_VERSION			(SATA_AHBDMA_BASE + 0x3fc)



extern int stm_sata_probe(void)
{
	int t, timeout;

	/* AHB bus wrapper setup */

	// SATA_AHB2STBUS_STBUS_OPC
	// 2:0  -- 100 = Store64/Load64
	// 4    -- 1   = Enable write posting
	// DMA Read, write posting always = 0
	/* opcode = Load4 |Store4 */
	writel(3, SATA_AHB2STBUS_STBUS_OPC);

	// SATA_AHB2STBUS_MESSAGE_SIZE_CONFIG
	// 3:0  -- 0111 = 128 Packets
	// 3:0  -- 0110 =  64 Packets
	/* WAS: Message size = 64 packet when 6 now 3 */
	writel(3, SATA_AHB2STBUS_MESSAGE_SIZE_CONFIG);

	// SATA_AHB2STBUS_CHUNK_SIZE_CONFIG
	// 3:0  -- 0110 = 64 Packets
	// 3:0  -- 0001 =  2 Packets
	/* WAS Chunk size = 2 packet when 1, now 0 */
	writel(2, SATA_AHB2STBUS_CHUNK_SIZE_CONFIG);

        // PC_GLUE_LOGIC
        // 7:0  -- 0xFF = Set as reset value, 256 STBus Clock Cycles
        // 8    -- 1  = Time out enabled
	// (has bit 8 moved to bit 16 on 7109 cut2?)
	/* time out count = 0xa0(160 dec)
	 * time out enable = 1
	 */
	if (pc_glue_logic_init)
	{
		writel(pc_glue_logic_init, SATA_PC_GLUE_LOGIC);
	}

	/* Clear initial Serror */
	writel(-1, SATA_SCR1);

#if 0
{
	/*
	 * Note: for this code to be executed, it is imperative
	 * that this SATA initialization takes place *AFTER*
	 * the serial console has been initialized!
	 */
	const unsigned sata_rev = readl(SATA_VERSIONR);
	const unsigned dmac_rev = readl(DMAC_COMP_VERSION);

	printf("info: SATA version %c.%c%c DMA version %c.%c%c\n",
		(int)(sata_rev >> 24) & 0xff,
		(int)(sata_rev >> 16) & 0xff,
		(int)(sata_rev >>  8) & 0xff,
		(int)(dmac_rev >> 24) & 0xff,
		(int)(dmac_rev >> 16) & 0xff,
		(int)(dmac_rev >>  8) & 0xff);
}
#endif

	/*
	 * Now we reset the SATA PHY.
	 */
	writel(0x301, SATA_SCR2);	/* issue phy wake/reset */
	readl(SATA_SCR0);		/* dummy read; flush */
	udelay(1000);			/* 1 ms  -  a guess */
	writel(0x300, SATA_SCR2);	/* phy wake/clear reset */

	/* For HDD detection issue */
	timeout = 25;			/* 2.5 ms */
	do
	{
		udelay(100);		/* 100 us */
		t = readl(SATA_SCR1);
		/* Wait till COMWAKE detected? */
		if (t & 0x40000)	/* bit #18, DIAG_W */
			break;
	} while (--timeout);

	/* now, deassert the deserializer reset */
	stm_sata_miphy_deassert_des_reset();

	/* wait for phy to become ready, if necessary */
	timeout = 25;			/* 5 sec */
	do
	{
		udelay(200000);		/* 200 ms */
		t = readl(SATA_SCR0);
		if ((t & 0xf) != 1)
			break;
	} while (--timeout);

	/* QQQ: we could now print the link status if we want ... */

	return 0;			/* Okay */
}


#endif	/* CONFIG_SH_STM_SATA */


