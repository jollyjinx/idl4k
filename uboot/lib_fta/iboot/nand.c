/*****************************************************************************

File name   :  nand.c

Description :  nand flash driver : initialize the NAND handler

 COPYRIGHT (C) Inverto 2009.

Date               Modification                                          Name
----               ------------                                          ----
 04/17/09          Created                                               FR

*****************************************************************************/
/* --- includes ----------------------------------------------------------- */
#include "iboot.h"
#include "asm/string.h"

#define CCMD    0
#define CINC    1
#define CJUMP   2
#define CSTOP   3
#define CDATA   4
#define CSPARE  5
#define CCHECK  6
#define CADDR   7

#define NAND_BASE               0xFE701000
#define NAND_FLEX               NAND_BASE+0x120 /* address of flex FIFO data          */
#define NAND_ADV_FLEX           NAND_BASE+0x300 /* address of advanced flex FIFO data */

#define ND_BOOTBANK_CFG         *(volatile unsigned int*) (NAND_BASE+0x000) /* EMI bootbank configuration       */
#define ND_RBn_STA              *(volatile unsigned int*) (NAND_BASE+0x004) /* EMI RN status                    */
#define ND_INT_EN               *(volatile unsigned int*) (NAND_BASE+0x010) /* EMI interrupt enable             */
#define ND_INT_STA              *(volatile unsigned int*) (NAND_BASE+0x014) /* EMI interrupt status             */
#define ND_INT_CLR              *(volatile unsigned int*) (NAND_BASE+0x018) /* EMI interrupt clear              */
#define ND_INT_EDGE_CFG         *(volatile unsigned int*) (NAND_BASE+0x01C) /* EMI interrupt edge config        */
#define ND_CTL_TIMING           *(volatile unsigned int*) (NAND_BASE+0x040) /* EMI control timing               */
#define ND_WEN_TIMING           *(volatile unsigned int*) (NAND_BASE+0x044) /* EMI WEN timing                   */
#define ND_REN_TIMING           *(volatile unsigned int*) (NAND_BASE+0x048) /* EMI REN timing                   */
#define ND_ZERO_REMAP           *(volatile unsigned int*) (NAND_BASE+0x04C) /* EMI REN timing                   */
#define ND_FLEXMODE_CFG         *(volatile unsigned int*) (NAND_BASE+0x100) /* EMI Flex mode config             */
#define ND_FLEX_MUXCTRL         *(volatile unsigned int*) (NAND_BASE+0x104) /* EMI Flex muxcontrol              */
#define ND_FLEX_CS_ALT          *(volatile unsigned int*) (NAND_BASE+0x108) /* EMI Flex CS alternate            */
#define ND_FLEX_DATAWRT_CFG     *(volatile unsigned int*) (NAND_BASE+0x10C) /* EMI Flex datawrite config        */
#define ND_FLEX_DATARD_CFG      *(volatile unsigned int*) (NAND_BASE+0x110) /* EMI Flex dataread config         */
#define ND_FLEX_CMD             *(volatile unsigned int*) (NAND_BASE+0x114) /* EMI command                      */
#define ND_FLEX_ADD_REG         *(volatile unsigned int*) (NAND_BASE+0x118) /* EMI Flex address                 */
#define ND_FLEX_DATA            *(volatile unsigned int*) (NAND_BASE+0x120) /* EMI Flex data                    */
#define ND_ADD_REG1             *(volatile unsigned int*) (NAND_BASE+0x1E0) /* EMI Flex data                    */
#define ND_ADD_REG2             *(volatile unsigned int*) (NAND_BASE+0x1E4) /* EMI Flex data                    */
#define ND_ADD_REG3             *(volatile unsigned int*) (NAND_BASE+0x1E8) /* EMI Flex data                    */
#define ND_SEQ_REG1             *(volatile unsigned int*) (NAND_BASE+0x200) /* EMI sequence reg1                */
#define ND_SEQ_REG2             *(volatile unsigned int*) (NAND_BASE+0x204) /* EMI sequence reg2                */
#define ND_SEQ_REG3             *(volatile unsigned int*) (NAND_BASE+0x208) /* EMI sequence reg3                */
#define ND_SEQ_REG4             *(volatile unsigned int*) (NAND_BASE+0x20C) /* EMI sequence reg4                */
#define ND_ADD                  *(volatile unsigned int*) (NAND_BASE+0x210) /* EMI address                      */
#define ND_EXTRA_REG            *(volatile unsigned int*) (NAND_BASE+0x214) /* EMI extra reg                    */
#define ND_CMND                 *(volatile unsigned int*) (NAND_BASE+0x218) /* EMI command                      */
#define ND_SEQ_CFG              *(volatile unsigned int*) (NAND_BASE+0x21C) /* EMI sequence config reg          */
#define ND_GEN_CFG              *(volatile unsigned int*) (NAND_BASE+0x220) /* EMI sequence config reg          */
#define ND_ECC(x)               *(volatile unsigned int*) (NAND_BASE+0x280+(x<<2)) /* EMI sequence status       */
#define ND_FIFO                 *(volatile unsigned int*) (NAND_BASE+0x300)

#define write_cmd(cs,nb,rbn,cmd)    ND_FLEX_CMD         =((cs)<<31)|(((nb)&3)<<28)|((rbn)<<27)|(cmd);
#define write_add(cs,nb,rbn,add)    ND_FLEX_ADD_REG     =((cs)<<31)|(1<<30)|(((nb)&3)<<28)|((rbn)<<27)|(add);
#define write_data(cs,nb,rbn)       ND_FLEX_DATAWRT_CFG =((cs)<<31)|(((nb)&3)<<28)|((rbn)<<27);
#define read_data(cs,nb,rbn)        ND_FLEX_DATARD_CFG  =((cs)<<31)|(((nb)&3)<<28)|((rbn)<<27);

#define DEBUG_NAND_ECC(x)       DEBUG_NAND(x)
#define DEBUG_NAND_V2F(x)       DEBUG_NAND(x)

typedef enum
{
    ECC_NO_ERROR,
    ECC_ERROR_RECOVERED,
    ECC_NOT_RECOVERABLE
} ecc_error_t;

static U32 page_data[NAND_PAGE_SIZE/4] ALIGN(32);
static U32 page_oob [NAND_OOB_SIZE /4] ALIGN(32);
static U8  bbt      [NAND_CHIP_SIZE/NAND_BLOCK_SIZE];
static ecc_error_t eccError;

static const U8 nand_eccTable[] =
{
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};

static inline I32 countbits(U32 byte)
{
    I32 res = 0;

    for (;byte; byte >>= 1)
    {
        res += byte & 0x01;
    }

    return res;
}


/* ========================================================================
   Name:        nand_readBlock
   Description: read a block of DATA from the FLEX interface or advanced flex 
                it is assumed that the function is aligned on 32 byte boundaries ideally
                and that the transfer is multiple of 32 bytes
   ======================================================================== */
static void nand_readBlock ( U32 *p, U32 size, U32 address )
{
    register double temp0 __asm__("dr0") ;
    I32 old_fpscr;
    U32 loop;

    __asm__ volatile("sts fpscr, %0" : "=r" (old_fpscr  ): ); 
    __asm__ volatile("lds %0, fpscr" ::"r" (0x00140000) );  /* i.e. FPSCR[19]=0; FPSCR[20]=1 */
    for(loop=0;loop<size; loop+=32)
    {
        __asm__("movca.l r0, @%0"  : : "r" (p) : "memory" );    /* pre-allocate the datacache */
        __asm__ volatile ("fmov @%0,%1"  : : "r"(address),"d"(temp0) : "memory" );
        __asm__ volatile ("fmov %1,@%0"  : : "r"(p)      ,"d"(temp0) : "memory" );  p+=2; 
        __asm__ volatile ("fmov @%0,%1"  : : "r"(address),"d"(temp0) : "memory" );
        __asm__ volatile ("fmov %1,@%0"  : : "r"(p)      ,"d"(temp0) : "memory" );  p+=2; 
        __asm__ volatile ("fmov @%0,%1"  : : "r"(address),"d"(temp0) : "memory" );
        __asm__ volatile ("fmov %1,@%0"  : : "r"(p)      ,"d"(temp0) : "memory" );  p+=2; 
        __asm__ volatile ("fmov @%0,%1"  : : "r"(address),"d"(temp0) : "memory" );
        __asm__ volatile ("fmov %1,@%0"  : : "r"(p)      ,"d"(temp0) : "memory" );  p+=2;
    }
    __asm__ volatile("lds %0, fpscr" : : "r" (old_fpscr) ); /* restore fpscr */
}

/* ========================================================================
   Name:        nand_eccCorrect
   Description: detect and correct 1 bit error in a 128, 256 or 512 byte block
                "p_data" is a pointer to the data.
                "old_ecc" is the proper ECC for the data.
                "new_ecc" is the ECC generated from the (possibly) corrupted data.
   ======================================================================== */
static void nand_eccCorrect(U8* dat, U8* read_ecc, U8* calc_ecc)
{
    U8 s0, s1, s2;

    s1 = calc_ecc[0] ^ read_ecc[0];
    s0 = calc_ecc[1] ^ read_ecc[1];
    s2 = calc_ecc[2] ^ read_ecc[2];

    if ((s0 | s1 | s2) == 0)
    {
        eccError = ECC_NO_ERROR;
        return;
    }

    /* Check for a single bit error */
    if( ((s0 ^ (s0 >> 1)) & 0x55) == 0x55 &&
        ((s1 ^ (s1 >> 1)) & 0x55) == 0x55 &&
        ((s2 ^ (s2 >> 1)) & 0x54) == 0x54) 
    {

        U32 byteoffs, bitnum;

        byteoffs  = (s1 << 0) & 0x80;
        byteoffs |= (s1 << 1) & 0x40;
        byteoffs |= (s1 << 2) & 0x20;
        byteoffs |= (s1 << 3) & 0x10;

        byteoffs |= (s0 >> 4) & 0x08;
        byteoffs |= (s0 >> 3) & 0x04;
        byteoffs |= (s0 >> 2) & 0x02;
        byteoffs |= (s0 >> 1) & 0x01;

        bitnum = (s2 >> 5) & 0x04;
        bitnum |= (s2 >> 4) & 0x02;
        bitnum |= (s2 >> 3) & 0x01;

        dat[byteoffs] ^= (1 << bitnum);

        DEBUG_NAND_ECC
        (
            uart_send_string("NAND : ECC Correction succeed\n\r");
        )
        eccError = ECC_ERROR_RECOVERED;
        return;
    }

    if(countbits(s0 | ((U32)s1 << 8) | ((U32)s2 <<16)) == 1)
    {
        DEBUG_NAND_ECC
        (
            uart_send_string("NAND : ECC Correction succeed\n\r");
        )
        eccError = ECC_ERROR_RECOVERED;
        return;
    }

    DEBUG_NAND_ECC
    (
        uart_send_string("NAND : ECC Correction failed\n\r");
    )
    eccError = ECC_NOT_RECOVERABLE;
}

/* ========================================================================
   Name:        nand_eccGen
   Description: generate ecc code (3 bytes) for 128,256 or 512 bytes
   ======================================================================== */
static void nand_eccGen(U8* dat, U8* ecc_code)
{
    U8 idx1,idx2,idx3,idx4,reg1=0, reg2=0, reg3=0, tmp1, tmp2,mask=0x40;
    U32 i;
    /* Build up column parity */
    for(i=0;i<256;i+=4)
    {   /* Get CP0 - CP5 from table */
        idx1 = nand_eccTable[*dat++];
        idx2 = nand_eccTable[*dat++];
        idx3 = nand_eccTable[*dat++];
        idx4 = nand_eccTable[*dat++];
        if (idx1 & mask)
        {
            reg3 ^=  (i+0);         reg2 ^= ~(i+0);
        }
        if (idx2 & mask)
        {
            reg3 ^=  (i+1);         reg2 ^= ~(i+1);
        }
        if (idx3 & mask)
        {
            reg3 ^=  (i+2);         reg2 ^= ~(i+2);
        }
        if (idx4 & mask)
        {
            reg3 ^=  (i+3);         reg2 ^= ~(i+3);
        }
        reg1^= idx1;
        reg1^= idx2;
        reg1^= idx3;
        reg1^= idx4;
    }
    reg1 = reg1&0x3f;

    /* Create non-inverted ECC code from line parity */
    tmp1  = (reg3 & 0x80) >> 0; /* B7 -> B7 */
    tmp1 |= (reg2 & 0x80) >> 1; /* B7 -> B6 */
    tmp1 |= (reg3 & 0x40) >> 1; /* B6 -> B5 */
    tmp1 |= (reg2 & 0x40) >> 2; /* B6 -> B4 */
    tmp1 |= (reg3 & 0x20) >> 2; /* B5 -> B3 */
    tmp1 |= (reg2 & 0x20) >> 3; /* B5 -> B2 */
    tmp1 |= (reg3 & 0x10) >> 3; /* B4 -> B1 */
    tmp1 |= (reg2 & 0x10) >> 4; /* B4 -> B0 */
    tmp2  = (reg3 & 0x08) << 4; /* B3 -> B7 */
    tmp2 |= (reg2 & 0x08) << 3; /* B3 -> B6 */
    tmp2 |= (reg3 & 0x04) << 3; /* B2 -> B5 */
    tmp2 |= (reg2 & 0x04) << 2; /* B2 -> B4 */
    tmp2 |= (reg3 & 0x02) << 2; /* B1 -> B3 */
    tmp2 |= (reg2 & 0x02) << 1; /* B1 -> B2 */
    tmp2 |= (reg3 & 0x01) << 1; /* B0 -> B1 */
    tmp2 |= (reg2 & 0x01) << 0; /* B7 -> B0 */

    /* Calculate final ECC code */
#ifdef CONFIG_MTD_NAND_ECC_SMC
    ecc_code[0] = ~tmp2;
    ecc_code[1] = ~tmp1;
#else
    ecc_code[0] = ~tmp1;
    ecc_code[1] = ~tmp2;
#endif
    ecc_code[2] = ((~reg1) << 2) | 0x03;
}


/* ========================================================================
   Name:        nand_decrypt2048
   Description: decrypt a 2176 sector to 2048
   ======================================================================== */
static void nand_decrypt2048 ( U32 src, U32 ecc)
{
    U8  newecc[3];
    U32 i;

    for (i=0; i<2048/256; i++,ecc+=3,src+=256)
    {
        nand_eccGen ((U8*)src,newecc);
        nand_eccCorrect((U8*)src , newecc,(U8*)ecc);
    }
}


/* ========================================================================
   Name:        nand_waitBusy
   Description: waits that NAND controller becomes idle
   ======================================================================== */
static void nand_waitBusy( void )
{
    U32 timeout=5000;
    while ( ((ND_RBn_STA&4) == 0)&&(timeout) )
    {
        timeout --;
    }

    DEBUG_NAND
    (
        if ( timeout == 0 )
        {
            uart_send_string("NAND : controller not idle before reading\n\r");
        }
    )
}

/* ========================================================================
   Name:        nand_cacheRead
   Description: perform a cache read operation

   ======================================================================== */
static void nand_cacheRead( U32 address )
{
    U32 addr1,addr2;
    addr1 = address & 0x7FF;
    addr2 = address >> 11;

    write_cmd(1,1,1,0x00)   /* cache read first step                         */
    write_add(1,2,1,addr1)  /* define column                                 */
    write_add(1,3,1,addr2)  /* define page                                   */
    write_cmd(1,1,1,0x30)   /* 0x30 read, 0x31 cache read cache read confirm */
    read_data(1,4,0)        /* prepare the read FIFO, we never know          */
    nand_waitBusy ();
}

/* ========================================================================
   Name:        nand_xorHw512
   Description: perform a hardware XORING of ECC 512. This is because the 
                hardware is performing automatically XORING in advanced flex
   ======================================================================== */
static void nand_xorHw512( void )
{
    page_oob[0] ^= ND_ECC(0);
    page_oob[4] ^= ND_ECC(1);
    page_oob[8] ^= ND_ECC(2);
    page_oob[12]^= ND_ECC(3);
}

/* ========================================================================
   Name:        nand_readPage
   Description: read 1 NAND Page
   ======================================================================== */
static void nand_readPage( U32 srcfsh )
{
    DEBUG_NAND(uart_send_string("NAND : Read Page\n\r");)

    nand_cacheRead  (srcfsh);                                   /* preload address of the block to read  */
    nand_readBlock  (page_data, NAND_PAGE_SIZE, NAND_FLEX );    /* read data and put in final location   */
    nand_readBlock  (page_oob , NAND_OOB_SIZE , NAND_FLEX );    /* read oob and put in separate location */

    /* software ECC : no need to XOR the bytes */
    nand_decrypt2048((U32)page_data,(U32)&page_oob[10]);        /* perform error correction      */
}

/* ========================================================================
   Name:        nand_readBbt
   Description: Read the Bad Block Table
   ======================================================================== */
static void nand_readBbt( void )
{
    int i;
    unsigned int j;
    U32 state;

    DEBUG_NAND(uart_send_string("NAND : Read BBT\n\r");)

    /* Clear the BBT */
    memset(bbt, 0, sizeof(bbt));

    /* Check the last 4 NAND Blocks */
    for(i=1; i<5; i++)
    {
        nand_readPage(NAND_CHIP_SIZE-i*NAND_BLOCK_SIZE);

        if( eccError != ECC_NOT_RECOVERABLE )
        {
            if( !strncmp((const char *)&page_oob[2], "Bbt0", 4) || !strncmp((const char *)&page_oob[2], "1tbB", 4))
            {
                for(j=0; j<sizeof(bbt)/16; j++)
                {
                    if( page_data[j] != 0xFFFFFFFF )
                    {
                        state = page_data[j] ^ 0xFFFFFFFF;
                        i = j*16;
                        while(state)
                        {
                            bbt[i++] = state & 0x3;
                            state >>= 2;
                        }
                    }
                }
                return;
            }
        }
    }
}

/* ========================================================================
   Name:        nand_burstRead
   Description: rebuild a file stored in the NAND
   ======================================================================== */
void nand_burstRead( U32 srcfsh, U32 dstmem, U32 size )
{
    U32 i;
    v2fNode_t *node;

    node = (v2fNode_t *) page_data;
    nand_cacheRead(srcfsh);                 /* preload adress of first block to read      */
    ND_FLEXMODE_CFG = 0x42;                 /* switch to advance flex mode                */
    ND_CMND         = 0x31313131;           /* prepare 0x31 : load next page in cache     */
    ND_SEQ_REG1     = (CINC << 0)|(1<<4)  | /* prefetch next page from cache      */
                      (CDATA<< 8)|(2<<12) | /* read 2048 + 64 bytes               */
                      (CJUMP<<16)|(0<<20) | /* read spare the given number of pages */
                      (CSTOP<<24)|(0<<28) ; /* when done : finish                 */
    ND_SEQ_REG2     = 0x00000000;
    ND_SEQ_REG3     = 0x00000000;
    ND_SEQ_REG4     = 0x00000000;
    ND_SEQ_CFG      = (1<<26)|(1<<25)|(0<<24)|(0<<16)|((size>>11)-1);  /* start for n pages */

    for(i=0;i<(size>>11);i++)
    {   /* read page and oob in separate area */
        nand_readBlock  (page_data, NAND_PAGE_SIZE, NAND_ADV_FLEX );    /* read data and put in final location   */
        nand_readBlock  (page_oob , NAND_OOB_SIZE , NAND_ADV_FLEX );    /* read oob and put in separate location */

        nand_xorHw512   ();                                   /* we need to XOR the bytes that were already XORED */
        nand_decrypt2048((U32)page_data,(U32)&page_oob[10]);  /* the ECC is stored in the last 24 Bytes (8 ECC)   */

        if( !bbt[i/(NAND_BLOCK_SIZE/NAND_PAGE_SIZE)] &&       /* The Block is valid             */
            (node->magic == V2F_MAGIC_BITMASK)      &&       /* The Page contains a V2F NODE   */
            (node->offset + node->dsize < size) )             /* We are staying inside the POOL */
        {
            if( node->compr == V2F_COMPR_NONE )
                memcpyf((void*)(dstmem+node->offset), node->data, node->dsize);
            else
            {
                U32 pageOff = 0;
                do
                {
                    if( node->compr == V2F_COMPR_FRAG )
                        memcpyf((void*)(dstmem+node->offset), node->data, node->dsize);
                    else if( node->compr == V2F_COMPR_LINEAR )
                        memset((void*)(dstmem+node->offset), node->linear, node->dsize);

                    pageOff += node->csize/4;
                    node = (v2fNode_t *)&page_data[pageOff];
                    if( (node->magic != V2F_MAGIC_BITMASK) || (node->offset + node->dsize > size) )
                    {
                        break;
                    }
                } while(pageOff < NAND_PAGE_SIZE/4);

                node = (v2fNode_t *)page_data;
            }
        }
    DEBUG_NAND_V2F
    (
        else
        {
            if( bbt[i/(NAND_BLOCK_SIZE/NAND_PAGE_SIZE)] && !(i&(NAND_BLOCK_SIZE/NAND_PAGE_SIZE-1)))
            {
                uart_send_string("NAND : Block ");
                uart_send_x32(srcfsh + i*NAND_PAGE_SIZE);
                uart_send_string(" skipped ( Bad Block )\n\r");
            }
            else if( node->magic == 0xFFFF )
            {
            }
            else if( node->magic != V2F_MAGIC_BITMASK )
            {
                uart_send_string("NAND : Block ");
                uart_send_x32(srcfsh + i*NAND_PAGE_SIZE);
                uart_send_string(" skipped ( Bad Node ");
                uart_send_x16(node->magic);
                uart_send_string(")\n\r");
            }
            else if( node->offset + node->dsize < size )
            {
                uart_send_string("NAND : Block ");
                uart_send_x32(srcfsh + i*NAND_PAGE_SIZE);
                uart_send_string(" skipped ( Bad Destination )\n\r");
            }

        }
    )
    }

    write_cmd (1,1,1,0x34);         /* exit cache mode */
    ND_FLEXMODE_CFG = 0x01;         /* switch back to normal flex mode */
}

/* ========================================================================
   Name:        nand_reset
   Description: perform reset
   ======================================================================== */
static void nand_reset( void )
{
    DEBUG_NAND(uart_send_string("NAND : Reset controller\n\r");)
    write_cmd(1,1,1,0xff)
    nand_waitBusy();
}

/* =======================================================================
   Name:        nand_init
   Description: initializes the NAND flash mini driver
   ======================================================================== */
void nand_burstInit( void )
{
    ND_BOOTBANK_CFG = 0x10;
    ND_FLEXMODE_CFG = (1<<6)|(1<<5)|(1<<3);     /* reset flex */
    ND_FLEXMODE_CFG = 0;                        /* release reset */
    ND_GEN_CFG      = (0<<18)|(0<<17)|(1<<16)|0;/* 0 extra cycle, 1 2048 page size , 0 no rate control */
    ND_INT_EN       = 0;                        /* no interrupt for now    */
    ND_INT_CLR      = 0;                        /* clear pending interrupt */
    ND_CTL_TIMING   = 0x0B0606; /*b0606 */      /* CE de assert, hold time  , We assertion time */
    ND_WEN_TIMING   = 0x000202; /*305*/         /* off time On time */
    ND_REN_TIMING   = 0x000202; /*404*/         /* off time ON time */
    ND_FLEX_CS_ALT  = 0;                        /* no bank selected */
    ND_FLEX_MUXCTRL = 0x2;                      /* no idea what it is */
    ND_FLEXMODE_CFG = (1<<6)|(0<<5)| 1 ;        /* enable flex mode, and reset ECC 512 */
    nand_reset  ();
    nand_readBbt();
}

U8 nand_checkBlock( U32 block )
{
    if(block > sizeof(bbt))
        return 4;

    return bbt[block];
}

/* ========================================================================
   Name:        nand_readSignature
   Description: read signature and stored to registers
   ======================================================================== */
void nand_readSignature ( U8 *device, U8 *manufacturer, U8 *sig1, U8 *sig2 )
{
    U32 read;
    write_cmd (1,1,1,0x90)
    write_add (1,1,1,0x00)
    read_data (1,4,1)
    read = ND_FLEX_DATA;
    if ( device )      *device       = read;
    if ( manufacturer) *manufacturer = read>>8;
    if ( sig1 )        *sig1         = read>>16;
    if ( sig2 )        *sig2         = read>>24;
}



