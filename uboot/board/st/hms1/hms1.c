/*
 * (C) Copyright 2004 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
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
#include <command.h>
#include <asm/soc.h>
#include <asm/stb7100reg.h>
#include <asm/io.h>
#include <asm/pio.h>

#if defined CFG_JFFS_CUSTOM_PART
#include <jffs2/jffs2.h>
#endif

void stb7100_clocks(void);

#define EPLD_ATAPI *(volatile unsigned char *)(0xa3900000)

#define LED *(volatile unsigned char *)(0xa2000000 + 0x00100010)

void flashWriteEnable(void)
{
}

void flashWriteDisable(void)
{
}

#define PIO_BASE  0xb8024000

static void configPIO(void)
{
  SET_PIO_ASC(PIO_BASE, 3, 2, 4, 5);
}

#if defined(CONFIG_CMD_IDE)

#ifdef CONFIG_SH_STB7100_IDE
static void stb7100mboard_init_ide(void)
{
  EPLD_ATAPI = 1; /* Enable ATAPI mode of EMI */
}
#endif

#endif

int board_init(void)
{
	unsigned long sysconf;
	/* Route UART2 instead of SCI to PIO4 */
	/* Set ssc2_mux_sel = 0 */
	sysconf = *STB7100_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<3);
	*STB7100_SYSCONF_SYS_CFG07 = sysconf;

	configPIO();

#if defined(CONFIG_CMD_IDE)
#ifdef CONFIG_SH_STB7100_IDE
	stb7100mboard_init_ide();
#endif
#ifdef CONFIG_SH_STB7100_SATA
	stb7100_sata_init();
#endif
#endif

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: HMS1"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	LED = 1;

	return 0;
}

#if defined CFG_JFFS_CUSTOM_PART

/*
 * jffs2_part_info - get information about a JFFS2 partition
 *
 * @part_num: number of the partition you want to get info about
 * @return:   struct part_info* in case of success, 0 if failure
 *
 * Reads env variable jff2part for partition info
 *
 */

static struct part_info part;
static int current_part = -1;

struct part_info* jffs2_part_info(int part_num) {
	void *jffs2_priv_saved = part.jffs2_priv;

	printf("jffs2_part_info: part_num=%i\n",part_num);

	if (current_part == part_num)
		return &part;

	/* u-boot partition                                                 */
	if(part_num==0){
		ulong offset, size=0;
		char *f;

		if ((f=getenv("jffs2part"))) {
			char *p;
			offset = simple_strtoul(f, &p, 16);
			size   = simple_strtoul(p+1, NULL, 16);
		}

		if (size) {
			memset(&part, 0, sizeof(part));

			part.offset=offset;
			part.size=size;

			/* Mark the struct as ready */
			current_part = part_num;

			printf("part.offset = 0x%08x\n",(unsigned int)part.offset);
			printf("part.size   = 0x%08x\n",(unsigned int)part.size);
		}
	}

	if (current_part == part_num) {
/*QQQ-HACK:	part.usr_priv = &current_part;	*/
		part.jffs2_priv = jffs2_priv_saved;
		return &part;
	}

	printf("jffs2_part_info: end of partition table\n");
	return 0;
}
#endif
