/*****************************************************************************

File Name   : definition.h

Description : Contains each protoypes used by lib_fta

 COPYRIGHT (C) FTA Communication technologies 2009.

*****************************************************************************/
/* Define to prevent recursive inclusion ---------------------------------- */
#ifndef DEFINITION_H__
#define DEFINITION_H__

/* Includes --------------------------------------------------------------- */
#include "fta/types.h"
#include <common.h>
#include <asm/cache.h>

#include <command.h>
#include "fta/osdlib.h"

/* Definitions from U-Boot ------------------------------------------------- */
/* ========================================================================= */
I32  do_usb       (cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);
I32  do_run       (cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);
I32  do_protect   (cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);
I32  do_fat_fsload(cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);
I32  do_flerase   (cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);
I32  do_mem_cp    (cmd_tbl_t *cmdtp, I32 flag, I32 argc, I8 *argv[]);

/* Definitions from lib_fta ------------------------------------------------ */
/* ========================================================================= */
U32  getKeyPressed(void);

void init_display (void);
void test_display (void);
I32  term_display (void);
void setVideoPio  (void);

void term_graphics(void);
void init_osd     (void);
void splash_text  (I8 *text);
void splash_update(I32 value);

void initFtaLib   (void);
void termFtaLib   (void);
I32  test_fta     (void);

void bootm_linux     (void *start);
void ssaCheck        (U8 *binary);
void launch_iboot_img(U32 addr);
void addDynamicArgs  (I8 *dst);

U32  board_getConfig (int useEnv);

void stx7105_spi_cs  (int val);
void stx7105_spi_data(int in);

void fp_init    (void);
void fp_write   (const I8 *text);
U8   fp_read_key(void);

#endif  /* DEFINITION_H__ */
