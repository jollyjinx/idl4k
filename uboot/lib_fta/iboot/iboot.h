/******************************************************************************/
/*                                                                            */
/* File name   : iboot.h                                                      */
/*                                                                            */
/* Description : Board generic definitions                                    */
/*                                                                            */
/* COPYRIGHT (C) FTA 2009                                                     */
/*                                                                            */
/* Date               Modification                 Name                       */
/* ----               ------------                 ----                       */
/* 10/05/09           Created                      E. FRADIN                  */
/*                                                                            */
/******************************************************************************/
#ifndef __IBOOT_H__
#define __IBOOT_H__

/* --- includes ----------------------------------------------------- */
#include "fta/definition.h"
#include <stdbool.h>

#ifdef CONFIG_SH_IDL52K
#include "idl52k_param.h"
#endif

#ifdef CONFIG_SH_IDL53K
#include "idl53k_param.h"
#endif

/* --- debug -------------------------------------------------------- */
#define USE_UART_DEBUG(x)

#define DEBUG_MSG(x)
#define DEBUG_MALLOC(x)
#define DEBUG_NOR(x)
#define DEBUG_NAND(x)
#define DEBUG_SSD(x)
#define DEBUG_I2C(x)
#define DEBUG_OP(x)


/* --- defines ------------------------------------------------------ */
#define ALIGN(x)        __attribute__((aligned(x)))
#define PACKED          __attribute__((packed))
#define UNUSED_PARAM    __attribute__((unused))

/* NAND definition */
/* =============== */
#define NAND_BLOCK_SIZE     0x20000
#define NAND_PAGE_SIZE      0x800
#define NAND_OOB_SIZE       0x40

/* ST40 OPTIMIZATION */
/* ================= */
#define ST40_OPTIM

/* HEADER Related data */
/* =================== */
#define BOOTLOADER_HEADER_SIZE      0x40
#define MODULE_PATTERN              0x64616548
#define MODULE_DESC_LENGTH          28

/* EEPROM Flag */
/* =========== */
#define FORCE_UPDATE_CODE   0x00445353

/* --- enums -------------------------------------------------------- */

/* HEADER Related data */
/* =================== */
typedef enum
{
    MODULE_1_SELF_TEST       = 0,
    MODULE_2_RECOVERY_LOADER = 1,
    MODULE_3_APPLICATION     = 2,
    MODULE_4_LOADER          = 3,
} module_t;

typedef enum
{
    MODULE_0        = 0x20303020, /* DA2                      */
    MODULE_1        = 0x20313020, /* MODULE_1_SELF_TEST       */
    MODULE_2        = 0x20323020, /* MODULE_2_RECOVERY_LOADER */
    MODULE_3        = 0x20333020, /* MODULE_3_APPLICATION     */
    MODULE_4        = 0x20343020, /* MODULE_4_LOADER          */
} application_t;

typedef enum
{
    ACTION_COPY     = 0x7970634D,
    ACTION_UNZIP    = 0x70697A55,
    ACTION_SET      = 0x7465734D,
    ACTION_HW_ARGS  = 0x67726148,
    ACTION_DOOP     = 0x706F6F44,
} action_t;

typedef enum
{
    OP_END       = 0,
    OP_WRITE_U8  = 1,
    OP_WRITE_U16 = 2,
    OP_WRITE_U32 = 3,
    OP_READ_U8   = 4,
    OP_READ_U16  = 5,
    OP_READ_U32  = 6,
    OP_OR_U8     = 7,
    OP_OR_U16    = 8,
    OP_OR_U32    = 9,
    OP_AND_U8    = 10,
    OP_AND_U16   = 11,
    OP_AND_U32   = 12,
    OP_ADD_U8    = 13,
    OP_ADD_U16   = 14,
    OP_ADD_U32   = 15,
    OP_SUB_U8    = 16,
    OP_SUB_U16   = 17,
    OP_SUB_U32   = 18,
    OP_IF_U8     = 19,
    OP_IF_U16    = 20,
    OP_IF_U32    = 21,
    OP_FORWARD   = 22,
    OP_BACKWARD  = 23
} op_code_t;

typedef enum
{
    IF_HIGHER    = 0x0000,
    IF_SMALLER   = 0x0100,
    OP_EQUALS    = 0x0200,
    OP_DIFFERENT = 0x0300
} if_code_t;

/* HW PIO Related data */
/* =================== */
typedef enum
{
  CFG_BMP_BRANDLESS        = 0,
  CFG_BMP_HDPLUS           = 1,
  CFG_BMP_SELEVISION       = 2,
  CFG_BMP_INVERTO          = 3,
  CFG_BMP_LAST             = 4
} inverto_pio_bmp_t;

/* V2F Related data */
/* ================ */
typedef enum
{
    V2F_COMPR_NONE   = 0,
    V2F_COMPR_FRAG   = 1,
    V2F_COMPR_LINEAR = 2,
} v2fCompr_t;


/* --- structures --------------------------------------------------- */
typedef struct
{
    U32   op;
    void *src;
    void *dst;
    U32   val;
} do_op_t;

typedef struct
{
    action_t        action;
    U32             src;
    U32             dst;
    U32             size;
    U8             *data;
} extraAction_t;

typedef struct
{
    U32             length;                         /* Provided by Nagra for each module */
    U32             pattern;                        /* Default Pattern used to identify this is a header */
    U32             address;
    application_t   application;                    /* Number of the module */

    U32             board;                          /* Board used (HW) */
    U32             releaseNumber;                  /* release number */
    U32             date;                           /* Date should be 0xYYYYMMDD (for day, month, year) */

    U32             nbExtraAction;                  /* Number of action */
    extraAction_t  *extraAction;                    /* Array of action_t structures (after authentication) */

    I8              description[MODULE_DESC_LENGTH];/* Human readable description of the module */
} moduleHeader_t;

/* HOLE_B Related data */
/* =================== */
#define SN_SIZE     16

typedef struct
{
  I8  SN[4];                  /* SNB\0 */
  I8  SerialNumber[SN_SIZE];
  I8  HV[4];                  /* HWV\0 */
  U32 HardwareVersion1;
  U32 HardwareVersion2;
  U32 HardwareVersion3;
  U32 HardwareVersion4;
} inverto_unique_id_type_t;

/* V2F Related data */
/* ================ */
#define V2F_MAGIC_BITMASK  0xA198

typedef struct
{
    unsigned short magic;
    unsigned char  linear;
    unsigned char  compr;
    unsigned int   offset;
    unsigned int   csize;
    unsigned int   dsize;
    unsigned char *data[];
} PACKED v2fNode_t;

typedef struct
{
    unsigned char random[48];
    unsigned char key[64];
    unsigned char digest[20];
} PACKED ssa_sign_t;

/* --- extern ------------------------------------------------------- */
extern const U8 conv[16];

/* --- MACRO Function ----------------------------------------------- */
#define ALLOCO(dst)     __asm__("movca.l r0, @%0" : : "r"(dst) : "memory")
#define PREFETCH(dst)   __asm__("pref @%0" : :"r"(dst) :"memory")


/* --- prototypes --------------------------------------------------- */
bool    forceUpdate    ( void );
void    nand_burstInit ( void );
void    nand_burstRead ( U32 srcfsh, U32 dstmem, U32 size );
void    memsetf        ( void *dst, I32 value, U32 lenght );
void    addDynamicArgs ( I8 *dst );
void    checkEnv       ( void );
void    tryOp          ( U32 op, void* src, void* dst, U32 val );
void    memcpyf        ( void *dst, void *src, U32 size );
int     uncompress     ( unsigned char *dest,unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int gzip );

/* --- inline functions --------------------------------------------- */
static inline unsigned int swap32(unsigned int data)
{
	__asm__("swap.b %0,%0\n"
			"swap.w %0,%0\n"
			"swap.b %0,%0\n"
			: "+r"(data));
	return data;
}

#endif  /* __IBOOT_H__ */


