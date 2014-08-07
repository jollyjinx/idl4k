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

#include "../iboot/iboot.h"

#define strtouc(x) ((x <  '0') ? 0           : \
                    (x <= '9') ? (x-'0'+ 0)  : \
                    (x <  'A') ? 0           : \
                    (x <= 'Z') ? (x-'A'+10)  : \
                    (x <  'a') ? 0           : \
                    (x <= 'z') ? (x-'a'+10)  : 0)

typedef struct
{
    unsigned char pk  [0x258];
    unsigned char csc [0x100];
    char          SNB [0x004];      /* SNB\0 */
    char          sn  [0x010];
    char          HWV [0x004];      /* HWV\0 */
    unsigned char hid [0x010];
    char          mac [0x011];
    char          ff0 [0x003];
    unsigned char egk [0x010];
    unsigned char cgk [0x010];
    unsigned char ff1 [0x04C];
} PACKED pls_t;

typedef struct
{
	unsigned char ff0 [0x040];
    unsigned char hdcp[0x134];
} PACKED private_keys_t;

static pls_t pls;
static private_keys_t keys;
static int isPlsValid = 0;

static void loadkey(char* name, unsigned char* buf, int buf_size)
{
    char address[12]  = {0};
    char max_size[12] = {0};

    char *usb [2] = {"usb","reset"};
    char *load[6] = {"fatload","usb", "0", address, name, max_size};

    sprintf(address,  "%X", buf);
    sprintf(max_size, "%X", buf_size);

    do_usb(NULL, 0, 2, usb);
    do_fat_fsload(NULL, 0, 6, load);
}

static void convertkey(const char* key, unsigned char* buf, int buf_size)
{
    int i =0;
    if(strlen(key) != 2*buf_size)
    {
        printf("Invalid size\n");
        return;
    }

    for(i=0; i<buf_size; i++)
    {
        buf[i] = (strtouc(key[2*i]) << 4) | (strtouc(key[2*i+1]) << 0);
    }
}

static void setkey(const char* key, char* buf, int buf_size)
{
    if(strlen(key) != buf_size)
    {
        printf("Invalid size\n");
        return;
    }
    memcpy(buf, key, buf_size);
}


int do_pls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    if( (argc == 2) || (argc == 3) )
    {
        if (strcmp(argv[0], "pls") == 0)
        {
            if(SCS_FA_HOLE_B_STOP-SCS_FA_HOLE_B_START != sizeof(pls_t))
            {
                printf("Struct pls_t has a wrong size\n");
                return 0;
            }

            if (strcmp(argv[1], "load") == 0)
            {
                memcpy(&pls, (void *)SCS_FA_HOLE_B_START, sizeof(pls_t));
                memcpy(pls.SNB, "SNB",   sizeof(pls.SNB));
                memcpy(pls.HWV, "HWV",   sizeof(pls.HWV));
                memcpy(&keys, (void *)PRIVATE_KEYS_START, sizeof(private_keys_t));
                isPlsValid = 1;
                return 0;
            }
            else
            {
                if(!isPlsValid)
                {
                    printf("use \"pls load\" first\n");
                    return 0;
                }

                if( argc == 3 )
                {
                    if (strcmp(argv[1], "PK") == 0)
                    {
                        loadkey(argv[2], pls.pk, sizeof(pls.pk));
                        return 0;
                    }
                    else if (strcmp(argv[1], "CSC") == 0)
                    {
                        loadkey(argv[2], pls.csc, sizeof(pls.csc));
                        return 0;
                    }
                    else if (strcmp(argv[1], "HDCP") == 0)
                    {
                        loadkey(argv[2], keys.hdcp, sizeof(keys.hdcp));
                        return 0;
                    }
                    else if (strcmp(argv[1], "SN") == 0)
                    {
                        setkey(argv[2], pls.sn, sizeof(pls.sn));
                        return 0;
                    }
                    else if (strcmp(argv[1], "MAC") == 0)
                    {
                        setkey(argv[2], pls.mac, sizeof(pls.mac));
                        return 0;
                    }
                    else if (strcmp(argv[1], "HW") == 0)
                    {
                        convertkey(argv[2], pls.hid, sizeof(pls.hid));
                        return 0;
                    }
                    else if (strcmp(argv[1], "EGK") == 0)
                    {
                        convertkey(argv[2], pls.egk, sizeof(pls.egk));
                        return 0;
                    }
                    else if (strcmp(argv[1], "CGK") == 0)
                    {
                        convertkey(argv[2], pls.cgk, sizeof(pls.cgk));
                        return 0;
                    }
                }
                else if (strcmp(argv[1], "info") == 0)
                {
                    printf("PLS info :\n");
                    print_buffer(SCS_FA_HOLE_B_START, &pls, 4, sizeof(pls_t)/4, 4);
                    printf("\n");
                    print_buffer(PRIVATE_KEYS_START, &keys, 4, sizeof(private_keys_t)/4, 4);
                    return 0;
                }

                else if (strcmp(argv[1], "clean") == 0)
                {
                    memset(&pls, 0xFF, sizeof(pls_t));
                    memcpy(pls.SNB, "SNB",   sizeof(pls.SNB));
                    memcpy(pls.HWV, "HWV",   sizeof(pls.HWV));
                    memset(&keys, 0xFF, sizeof(private_keys_t));
                    return 0;
                }

                else if (strcmp(argv[1], "save") == 0)
                {
                    unsigned char *block = (unsigned char *)malloc(CFG_ENV_SECT_SIZE);
                    if(block)
                    {
                        char address[12] = {0};
                        char *protect[]  = {"protect", "off", "all"};
                        sprintf(address, "%X", block);
                        do_protect(NULL, 0, 3, protect);

                        // Save the PLS sector
                        {
                            char *erase[] = {"erase", "0xA0010000", "0xA001FFFF"};
                            char *copy [] = {"cp.b", address, "0xA0010000", XSTR(CFG_ENV_SECT_SIZE)};

                            memcpy(&block[0x0000], (void*)0x10000, 0xFC00);
                            memcpy(&block[0xFC00], &pls, sizeof(pls_t));
                            do_flerase(NULL, 0, 3, erase);
                            do_mem_cp (NULL, 0, 4, copy );
                        }

                        // Save the private keys sector
                        {
                            char *erase[] = {"erase", "0xA0040000", "0xA004FFFF"};
                            char *copy [] = {"cp.b", address, "0xA0040000", XSTR(CFG_ENV_SECT_SIZE)};

                            memcpy(block, (void*)0x40000, CFG_ENV_SECT_SIZE);
                            memcpy(block, &keys, sizeof(private_keys_t));
                            do_flerase(NULL, 0, 3, erase);
                            do_mem_cp (NULL, 0, 4, copy );
                        }
                        free(block);
                    }
                    else
                    {
                        printf("Saving the PLS failed\n");
                    }
                    return 0;
                }
            }
        }
    }

    printf ("Usage:\n%s\n", cmdtp->usage);
    return 1;
}

U_BOOT_CMD(
    pls,	3,	2,	do_pls,
    "pls     - configure the PLS section\n",
    "     - configure the PLS section\n\n"
    "    pls load\n"
    "         - read the PLS from Flash (must be done first)\n"
    "    pls info\n"
    "         - display the PLS\n"
    "    pls clean\n"
    "         - clean the PLS (setting its content to 0xFF)\n"
    "    pls save\n"
    "         - save the PLS in Flash\n"
    "    pls [PK|CSC|SN|HW|MAC|EGK|CGK|HDCP] #value\n"
    "         - program the specified key\n"
    "\n"
);
