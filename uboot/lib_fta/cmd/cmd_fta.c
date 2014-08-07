/*
 * (C) Copyright 2010
 * Adam Wilczynski, Inverto Digital Labs, adam.wilczynski@inverto.tv.
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

#include "fta/definition.h"
#include "fta/display.h"

#include <exports.h>
#include <flash.h>

#include "../iboot/iboot.h"

extern flash_info_t flash_info[];
static int test_nor(void)
{
    int result=-1;
    int bank;

    printf("Starting NOR test\n");
    for (bank=0; bank < CFG_MAX_FLASH_BANKS; bank++)
    {
        ushort last_sector      = flash_info[bank].sector_count-1;
        ulong last_sector_first = flash_info[bank].start[last_sector];
        ulong last_sector_last  = flash_info[bank].start[0]+flash_info[bank].size-1;
        ulong last_sector_size  = last_sector_last-last_sector_first+1;
        int i, res=-1;
        char *buff=NULL;

        buff = (char*)malloc(last_sector_size*2);
        if (buff)
        {
            int protect = flash_info[bank].protect[last_sector];
            for (i=0; i<last_sector_size; i++)
            {
                buff[last_sector_size+i] = ((char*)last_sector_first)[i];
            }
            if (protect)
                flash_sect_protect(0,last_sector_first,last_sector_last);
            if (!flash_sect_erase(last_sector_first,last_sector_last))
            {
                for (i=0; i<last_sector_size; i++)
                {
                    buff[i] = ((i*37+13)%0xff);
                }
                if (!flash_write(buff,last_sector_first,last_sector_size))
                {
                    /* verify */
                    for (i=0; i<last_sector_size; i++)
                    {
                        if (buff[i] != ((char*)last_sector_first)[i])
                        break;
                    }
                    if (i == last_sector_size)
                    {
                        res = 0;
                        /* restore sector content */
                        flash_sect_erase(last_sector_first,last_sector_last);
                        flash_write(buff+last_sector_size,last_sector_first,last_sector_size);
                        if (protect)
                            flash_sect_protect(1,last_sector_first,last_sector_last);
                    }
                }
            }
            free(buff);
        }
        if (res)
            break;
    }
    if (bank == CFG_MAX_FLASH_BANKS)
        result = 0;
    printf("    NOR test %s\n", result ? "failed" : "passed");
    return result;
}

#include <i2c.h>
static int test_i2c(void)
{
    int result=-1;
    int j;

    printf("Starting I2C test\n    Valid chip addresses:");
    for (j = 0; j < 128; j++)
    {
        if (i2c_probe(j) == 0)
        {
            printf(" %02X", j<<1);
            result = 0;
        }
    }
    printf("\n    I2C test %s\n", result ? "failed" : "passed");

    return result;
}

static int test_eeprom(void)
{
    int result=-1;
    U8 back_buffer[16];

    printf("Starting EEPROM test\n");

    if (!eeprom_read(CFG_I2C_EEPROM_ADDR,0,back_buffer,sizeof(back_buffer)))
    {
        U8 test_buffer[16];
        U8 i;

        for (i=0; i < sizeof(test_buffer); i++)
        {
          test_buffer[i] = ((i*37+13)%0xff);
        }
        if (!eeprom_write(CFG_I2C_EEPROM_ADDR,0,test_buffer,sizeof(test_buffer)))
        {
            udelay(25000);
            if (!eeprom_read(CFG_I2C_EEPROM_ADDR,0,test_buffer,sizeof(test_buffer)))
            {
                for (i=0; i < sizeof(test_buffer); i++)
                {
                    if (test_buffer[i] != ((i*37+13)%0xff))
                        break;
                }
                if (i == sizeof(test_buffer))
                {
                    result = 0;
                }
            }
            eeprom_write(CFG_I2C_EEPROM_ADDR,0,back_buffer,sizeof(back_buffer));
            udelay(25000);
        }
    }
    printf("    EEPROM test %s\n", result ? "failed" : "passed");
    return result;
}

#include <asm/io.h>
#include <asm/pio.h>

#define NUM_I2C_DEVICES 2

typedef struct
{
    U8  addr;
    U16 reg;
} demod_t;

static int test_tuner(void)
{
    const demod_t demod[NUM_I2C_DEVICES] =
    {
        {0xD0, 0xF100},
        {0x38, 0xF000}
    };

    int result=-1;
    U8 demod_id = 0;
    U8 idx = 0;

    printf("Starting TUNER test\n");

    SET_PIO_PIN(PIO_PORT(1), 2, STPIO_OUT);
    STPIO_SET_PIN(PIO_PORT(1), 2, 0);
    udelay(10000);
    STPIO_SET_PIN(PIO_PORT(1), 2, 1);
    udelay(10000);

    /* try to detect which Demod is connected to the I2C bus */
    for( idx=0; idx<NUM_I2C_DEVICES; idx++ )
    {
        /* try to read id register - it should succeed if the device exist */
        if ( !i2c_read(demod[idx].addr >> 1, demod[idx].reg, sizeof( demod[0].reg ), &demod_id, 1) )
        {
            printf("    DEMOD id 0x%02x\n", demod_id);

            STPIO_SET_PIN(PIO_PORT(1), 2, 0);
            udelay(10000);

            /* try to read id register - it should fail while demod in reset*/
            if ( i2c_read(demod[idx].addr >> 1, demod[idx].reg, sizeof( demod[0].reg ), &demod_id, 1) )
                result = 0;

            STPIO_SET_PIN(PIO_PORT(1), 2, 1);
            break;
        }
    }

    printf("    TUNER test %s\n", result ? "failed" : "passed");
    return result;
}

static int test_fp(void)
{
    printf("Starting FRONT PANEL test\n");
    printf("    Do not Press any key...  ");
    while( getKeyPressed() != FP_KEY_NONE );
    printf("OK\n");
    printf("    Press the UP key...      ");
    while( getKeyPressed() != FP_KEY_UP );
    printf("OK\n");
    printf("    Press the DOWN key...    ");
    while( getKeyPressed() != FP_KEY_DOWN );
    printf("OK\n");
    printf("    Press the POWER key...   ");
    while( getKeyPressed() != FP_KEY_POWER );
    printf("OK\n");

    printf("    FRONT PANEL test passed\n");
    return 0;
}

static int test_usb(U32 base)
{
    *((DU32*)(base+0x14))    = 0;
    udelay(100000);
    *((DU32*)(base+0xffe18)) = 5;
    udelay(100000);
    *((DU32*)(base+0xffe10)) = 0x80b01;
    udelay(100000);
    *((DU32*)(base+0xffe50)) = 1;
    udelay(100000);
    *((DU32*)(base+0xffe54)) = 0x1000;
    udelay(100000);
    *((DU32*)(base+0xffe54)) |=0x100;
    udelay(100000);
    *((DU32*)(base+0xffe54)) &= 0xfffffeff;
    udelay(100000);
    // -- suspend --
    *((DU32*)(base+0xffe54)) |= 0x80;
    udelay(100000);
    // -- test mode 4 --
    *((DU32*)(base+0xffe10)) &= 0xffffffce;
    udelay(100000);
    *((DU32*)(base+0xffe54)) &= 0xfff0ffff;
    *((DU32*)(base+0xffe54)) |= 0x00040000;

    return 0;
}

I32 test_fta(void)
{
#if defined(CONFIG_SELF_TEST_NOR)
    if( test_nor() )
        return -1;
#endif
#if defined(CONFIG_SELF_TEST_I2C)
    if( test_i2c() )
        return -1;
#endif
#if defined(CONFIG_SELF_TEST_TUNER)
    if( test_tuner() )
        return -1;
#endif
#if defined(CONFIG_SELF_TEST_EEPROM)
    if( test_eeprom() )
        return -1;
#endif
    return 0;
}

typedef struct
{
    char* hw;
    char* model;
    char* config;
    char* name;
} device_t;

static const device_t device[] =
{
    {"1400156A180000061001301000000000", "3500", "6", "Zappix"				}, /* GUI SMART   +  7 LED 			*/
    {"1400156A180000065001301000000000", "3500", "2", "CX10"				}, /* GUI SMART   + 16 SEG 			*/
    {"1400156A180000065001301000000000", "3500", "A", "VX10"				}, /* GUI SMART   + 16 SEG + WIFI 	*/
    {"1400156A190000064001301000000000", "3500", "7", "Vbox Essential"		}, /* GUI INVERTO +  7 LED 			*/
    {"1410156A180000065001301000000000", "3500", "B", "Vbox Web Edition"	}, /* GUI INVERTO + 16 SEG + WIFI 	*/
    {"1410256A180000065001301000000000", "3500", "B", "Vbox Web Edition +"	}, /* GUI INVERTO + 16 SEG + WIFI 	*/

    {"1420257A100000067010301000000000", "3600", "2", "SELEVISION"			}, /* GUI SELEVISION 				*/
    {"1420257A100000066001301000000000", "3600", "3", "Vbox II"				}, /* GUI INVERTO 					*/

    {"1430256A190000061001301000000000", "3700", "7", "ROBERTS"				}  /* GUI INVERTO + 7 LED 			*/
};

static void printModel(void)
{
    int i;
    for(i=0; i<sizeof(device)/sizeof(device_t); i++)
    {
        printf("[%d] %s\n", i, device[i].name);
    }
}

static void setModel(const char *model)
{
    if( (model[0] >= '0') && (model[0] <= '9') )
    {
        int i = model[0] - '0';
        printf("Setting the Model as a %s\n", device[i].name);
        setenv("hw",     device[i].hw    );
        setenv("model",  device[i].model );
        setenv("config", device[i].config);
    }
}

static void printChipset(void)
{
    printf("\n");
    printf("Chipset Information\n");
    printf("  Processor STi%hX\n", *(unsigned short*)0xFE00D08E);
    printf("  Version   %cU%c\n", *(char*)0xFE00D08D, 'A' + (*(char*)0xFE001003 >> 4) );
    printf("  Extra ID  %.8X\n", *(unsigned int*)0xFE001004);
    printf("\n");
}

int do_test_fta (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    if(argc==2)
    {
        if (strcmp(argv[1], "test") == 0)
        {
            test_fta();
            return 0;
        }
        else if (strcmp(argv[1], "env") == 0)
        {
            gd->env_valid = 0;
            env_relocate();
            return 0;
        }
        else if (strcmp(argv[1], "config") == 0)
        {
            checkEnv();
            return 0;
        }
        else if (strcmp(argv[1], "gfx") == 0)
        {
            init_osd();
            splash_update(64);
            return 0;
        }
        else if (strcmp(argv[1], "model") == 0)
        {
            printModel();
            return 0;
        }
        else if (strcmp(argv[1], "chipset") == 0)
        {
        	printChipset();
            return 0;
        }
    }

    if(argc==3)
    {
        if (strcmp(argv[1], "test") == 0)
        {
            if (strcmp(argv[2], "NOR") == 0)
            {
                test_nor();
                return 0;
            }
            else if (strcmp(argv[2], "I2C") == 0)
            {
                test_i2c();
                return 0;
            }
            else if (strcmp(argv[2], "EEPROM") == 0)
            {
                test_eeprom();
                return 0;
            }
            else if (strcmp(argv[2], "TUNER") == 0)
            {
                test_tuner();
                return 0;
            }
            else if (strcmp(argv[2], "FP") == 0)
            {
                test_fp();
                return 0;
            }
            else if (strcmp(argv[2], "DENC") == 0)
            {
                test_display();
                return 0;
            }
            else if (strcmp(argv[2], "USB0") == 0)
            {
                test_usb(CFG_USB0_BASE);
                return 0;
            }
            else if (strcmp(argv[2], "USB1") == 0)
            {
                test_usb(CFG_USB1_BASE);
                return 0;
            }
        }
        else if (strcmp(argv[1], "model") == 0)
        {
            setModel(argv[2]);
            return 0;
        }
    }

    if(argc==6)
    {
        if (strcmp(argv[1], "doop") == 0)
        {
            tryOp( ( U32 )simple_strtoul(argv[2], NULL, 0),
                   (void*)simple_strtoul(argv[3], NULL, 0),
                   (void*)simple_strtoul(argv[4], NULL, 0),
                   ( U32 )simple_strtoul(argv[5], NULL, 0));
            return 0;
        }
    }


    printf ("Usage:\n%s\n", cmdtp->usage);
    return 1;
}

U_BOOT_CMD(
    fta,	7,	1,	do_test_fta,
    "fta     - Run FTA specific functions\n",
    "     - Run FTA specific functions\n\n"
    "    fta env\n"
    "         - Delete the Environment-variables\n"
    "    fta test [NOR|I2C|EEPROM|TUNER|FP|DENC|USBn]\n"
    "         - Run the self-test FTA function\n"
    "    fta config\n"
    "         - Display the STB configuration\n"
    "    fta doop\n"
    "         - Generate a cmd to be interpreted\n"
    "    fta gfx\n"
    "         - Reload the Graphics\n"
    "    fta model [nb]\n"
    "         - Set the environment for a given model\n"
    "    fta chipset\n"
    "         - Display information about the Chipset\n"
    "\n"
);
