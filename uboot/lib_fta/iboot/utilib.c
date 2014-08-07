/*****************************************************************************

File name   :  util.c

Description :  Contains some function witch will be rewritten in ASM code

 COPYRIGHT (C) FTA Communication technologies 2009.

Date               Modification                                          Name
----               ------------                                          ----
10/14/09           Created                                               FR

*****************************************************************************/
#include "iboot.h"
#include <version.h>
#include "linux/ctype.h"

typedef union mixed
{
    double   tempx ;
    U32      val[2] ;
} conv_t ;

#include "../zlib/zlib.h"	/* main header */
#define BASE            65521UL    /* largest prime smaller than 65536 */
#define NMAX            5552       /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

/* ========================================================================
   Name:        adler32f
   Description: perform adler32

   ======================================================================== */
unsigned long  adler32f(unsigned long adler, const unsigned char *buf, unsigned int len)
{
    unsigned long s1 = adler & 0xffff , s2 = (adler >> 16) & 0xffff, k;
#if defined(ST40_OPTIM)
    unsigned char * bufadv;
#endif
    if (buf == Z_NULL) return 1L;	/* initial value */
#if defined(ST40_OPTIM)
    bufadv = (unsigned char*) (((int)buf) );
    __asm__("pref @%0" : :"r"(bufadv) :"memory");
    bufadv+=32;
#endif
    while (len > 0)
    {
        k = len < NMAX ? (int)len : NMAX;
        len -= k;
        while (k >= 8)
        {
#if defined (ST40_OPTIM)
            __asm__("pref @%0" : :"r"(bufadv) :"memory"); /* prefetch without blocking the CPU */
            bufadv += 8;
#endif
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;

            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            k -=8;
        }
        while (k--)	/* do the remaining ones */
        {
            s1 += *buf++;   s2 += s1;
        }
        s1 = s1 % BASE;	/* modulo */
        s2 = s2 % BASE;	/* modulo */
    }
    return (s2 << 16) | s1;
}

/* ========================================================================
   Name:        append
   Description: Append a length-limited, '\0'-terminated string to another

   ======================================================================== */
static I8 * append(I8 *dest, const I8 *src, I32 count)
{
    if (count)
    {
        while (*dest)
        {
            dest++;
        }
        while ((*dest++ = *src++))
        {
            if (--count == 0)
            {
                *dest = '\0';
                break;
            }
        }
    }

    return dest;
}


typedef enum
{
    NO_FLAG              = 0,
    REMOVE_TRAILING_ZERO = 1
} arg_t;

static I8 *setStrArg(I8* dst, const I8* arg, I8* value, U32 len)
{
    if(arg)
    {
        *dst++ = ' ';
        dst = append(dst, arg, strlen(arg));
        *dst++ = '=';
    }

    if(value && len)
    {
        dst = append(dst, value, len);
    }

    return dst;
}

static I8 *setU32Arg(I8* dst, const I8* arg, U32 value, I32 flag)
{
    U32 msk = 0xF0000000;

    dst = setStrArg(dst, arg, NULL, 0);

    if(flag & REMOVE_TRAILING_ZERO)
    {
        for(; msk>0; msk >>= 4)
        {
            if( value & msk )
                break;
        }
    }

    for(;msk>0; msk >>= 4)
    {
        *dst++ = conv[(value&msk)/(msk/0xF)];
    }

    return dst;
}

/* ========================================================================
   Name:        addDynamicArgs
   Description: Append some dynamics bootargs at address dst

   ======================================================================== */
void addDynamicArgs(I8 *dst)
{
    inverto_unique_id_type_t    *iuid;
    moduleHeader_t              *hdr;
    I8                          *ptr;
    U32                          tmp;

    iuid = (inverto_unique_id_type_t *) IUID_START;
    hdr  = (moduleHeader_t *)           (MODULE_4_START);
    /* Dynamics args have to be placed after the default one */
    dst += strlen(dst);

    ptr = getenv ("bootargs");
    if(ptr)
    {
        dst = setStrArg(dst, NULL, ptr, strlen(ptr));
    }

    /* Add the Model Type to the Bootargs */
    ptr = getenv("model");
    tmp = ptr ? simple_strtoul(ptr ,NULL, 16) : MODEL_TYPE;
    dst = setU32Arg(dst, "model", tmp, REMOVE_TRAILING_ZERO);

    /* Add the MAC address to the Bootargs and configure the PHY */
    ptr = getenv("ethaddr");
    if(!ptr) ptr = (I8*)MAC_START;
    dst = setStrArg(dst, "mac", ptr, 17);

    /* Add the bootloader version */
    ptr = U_BOOT_VERSION;
    dst = setStrArg(dst, "uboot", &ptr[7], strlen(U_BOOT_VERSION)-7);

    /* Add the Loader version */
    if(hdr->pattern == MODULE_PATTERN)
    {
        dst = setU32Arg(dst, "loader", hdr->releaseNumber, REMOVE_TRAILING_ZERO);
    }

    /* Add the HW configuration */
    dst = setU32Arg(dst, "config", board_getConfig(1), REMOVE_TRAILING_ZERO);

    /* Add the SN Number */
    ptr = getenv("sn");
    if(ptr)
    {
        tmp = strlen(ptr);
    }
    else
    {
        ptr = iuid->SerialNumber;
        for(tmp=0;tmp<SN_SIZE; tmp++)
        {
            if( !isalnum(ptr[tmp]) )
                break;
        }
    }
    dst = setStrArg(dst, "sn", ptr, tmp);

    /* Add the HW Number */
    ptr = getenv("hw");
    if(ptr)
    {
        dst = setStrArg(dst, "hw", ptr, strlen(ptr));
    }
    else
    {
        dst = setU32Arg(dst, "hw", swap32(iuid->HardwareVersion1), NO_FLAG);
        dst = setU32Arg(dst, NULL, swap32(iuid->HardwareVersion2), NO_FLAG);
        dst = setU32Arg(dst, NULL, swap32(iuid->HardwareVersion3), NO_FLAG);
        dst = setU32Arg(dst, NULL, swap32(iuid->HardwareVersion4), NO_FLAG);
    }
}


/* ========================================================================
   Name:        memcpyf
   Description: perform a memcpy, aligned on 32 bits

   ======================================================================== */
void memcpyf ( void *dst, void *src, U32 size )
{
    U32 s,d,i,items,align,tmp1,tmp0,pref;
    s   = (U32) src;
    d   = (U32) dst;
    pref= (U32) src;
    PREFETCH(pref);    pref+=32;

    /* PART 1 : align to 32 bytes boundaries */
    items = size < 128 ? size : ((-d)&31);
    size -= items;

    for(i=0;i<items;i++)
    {   /* align to words on destination */
        *(U8*)d++ = *(U8*)s++;
    }

    if ( size==0 )
    {
        return;
    }

    PREFETCH(pref);    pref   +=64;
    align = s&3;          /* align source to beginning of chunk */
    s     = s-align;      /* align source to beginning of chunk */
    items = (size>>5);
    size  = size &31;

    /* PART 2 : copy aligned words based on destination alignment       */
    /* 3 possible optimzations are available 0, 16 bites and 8-24 bits */
    if ( align == 0)
    {

#if 1
        if ( (s & 0x7) == 0  )
        {   /* let's take-out the big weapon ! */
            I32 old_fpscr;
            PREFETCH(pref);   pref+=32; /* need a bit more of pipeline */
            __asm__ ("sts fpscr, %0" : "=r" (old_fpscr  ): );
            __asm__ ("lds %0, fpscr" ::"r" (0x00140000) );  // i.e. FPSCR[19]=0; FPSCR[20]=1
            __asm__  ("fmov @%0,dr0"  : : "r"(s): "memory" );  s+=8;
            __asm__  ("fmov @%0,dr2"  : : "r"(s): "memory" );  s+=8;
            __asm__  ("fmov @%0,dr4"  : : "r"(s): "memory" );  s+=8;
            __asm__  ("fmov @%0,dr6"  : : "r"(s): "memory" );  s+=8;

            for(i=0;i<items;i++)
            {
                ALLOCO(d);
                __asm__  ("fmov dr0,@%0"  : : "r"(d): "memory" );  d+=8;
                __asm__  ("fmov dr2,@%0"  : : "r"(d): "memory" );  d+=8;
                __asm__  ("fmov dr4,@%0"  : : "r"(d): "memory" );  d+=8;
                __asm__  ("fmov dr6,@%0"  : : "r"(d): "memory" );  d+=8;
                __asm__  ("fmov @%0,dr0"  : : "r"(s): "memory" );  s+=8;
                __asm__  ("fmov @%0,dr2"  : : "r"(s): "memory" );  s+=8;
                __asm__  ("fmov @%0,dr4"  : : "r"(s): "memory" );  s+=8;
                __asm__  ("fmov @%0,dr6"  : : "r"(s): "memory" );  s+=8;
                PREFETCH(pref);   pref+=32;
            }
            __asm__ ("lds %0, fpscr" : : "r" (old_fpscr) ); // restore fpscr
            s-=32;
        }

        else
#endif

        for(i=0;i<items;i++)
        {   /* move 8*4 = 32  bytes at a time */
            ALLOCO(d);

            PREFETCH(pref);   pref+=32;

            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
            *(U32*) d  = *(U32*) s ;       d+=4; s+=4;
        }
    }
    else if ( align == 2 )
    {       /* move 8*4 = 32 bytes at a time, using extract 16 function */
        tmp0 =*(U32*)s;  s+=4;  /* read first chunk */
        for(i=0;i<items;i++)
        {
            PREFETCH(pref);   pref+=32;
            ALLOCO(d);

            tmp1                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp1) , "r"(tmp0)  );
            *(U32 *) d  = tmp0; d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp0) , "r"(tmp1)  );
            *(U32 *) d  = tmp1; d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp1) , "r"(tmp0)  );
            *(U32 *) d  = tmp0; d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp0) , "r"(tmp1)  );
            *(U32 *) d  = tmp1; d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp1) , "r"(tmp0)  );
            *(U32 *) d  = tmp0; d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp0) , "r"(tmp1)  );
            *(U32*) d  = tmp1; d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp1) , "r"(tmp0)  );
            *(U32*) d  = tmp0; d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            __asm__ ("xtrct %0,%1" :: "r"(tmp0) , "r"(tmp1)  );
            *(U32*) d  = tmp1; d+=4;
        }
    }
    else
    {
        U32 ar = (align <<3);
        U32 al = 32 - ar;
        tmp0 =*(U32*)s;  s+=4;  /* read first chunk */

        for(i=0;i<items;i++)

        {   /* move 32 bytes at a time, peform 8 24 bit shifting & masking */

            PREFETCH(pref);   pref+=32;
            ALLOCO(d);
            tmp1                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp0 >> ar) | (tmp1<<al);  d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp1 >> ar) | (tmp0<<al);  d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp0 >> ar) | (tmp1<<al);  d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp1 >> ar) | (tmp0<<al);  d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp0 >> ar) | (tmp1<<al);  d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp1 >> ar) | (tmp0<<al);  d+=4;
            tmp1                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp0 >> ar) | (tmp1<<al);  d+=4;
            tmp0                 = *(U32*) s ;       s+=4;
            *(U32*) d  = (tmp1 >> ar) | (tmp0<<al);  d+=4;
        }
    }

    /* PART 3 : last remaining bytes  */
    for(i=0;i<size;i++)
    {
        *(U8 *) d++ = *(U8 *)s++;
    }
}


/* ========================================================================
   Name:        memsetf
   Description: mem version of memset

   ======================================================================== */
void memsetf ( void * d, I32 p8, U32 size )
{
    unsigned int  pw = (unsigned int)d, p32,i,old_fpscr,items;
    register double temp0 __asm__("dr0") ;
    conv_t   c;

    if (size<12)
    {
        for(i=0;i<size;i++,*(unsigned char*)pw++ = p8);
        return;
    }
    PREFETCH(d);
    p32  = p8 & 0xff;
    p32 |= (p32<<8) ;
    p32 |=(p32<<16) ; /* convert byte to word */
    if (size <257)
    {
        items = ((-pw)&3); size -= items;
        for ( i=0;i<items    ; i++,*(unsigned char*)pw++=p32);
        for ( i=0;i<(size>>4); i++)
        {
            *(unsigned int *) pw = p32; pw+=4;
            *(unsigned int *) pw = p32; pw+=4;
            *(unsigned int *) pw = p32; pw+=4;
            *(unsigned int *) pw = p32; pw+=4;
        }
        for ( i=0;i<((size>>2)&3 ); i++, *(unsigned int *) pw = p32, pw+=4);
        for ( i=0;i<(size&3)   ; i++,*(unsigned char*)pw++=p32 );
        return;
    }
    /* PART 1 : until alignment to data word   */
    c.val[0]  = p32;    c.val[1]  = p32;    temp0 = c.tempx ;
    __asm__ volatile("sts fpscr, %0" : "=r" (old_fpscr  ): );
    __asm__ volatile("lds %0, fpscr" ::"r" (0x00140000) );
    items = ((-pw)&0x3);   size -= items;
    for(i=0;i<items;i++,*(unsigned char*)pw++=p32);
    /* PART 2 : until alignment of cache line : 32 bytes boundaries */
    items = ((-pw)&0x1C);  /* number of words to write until alignemnt 0-8-16-24- */
    size -= items ;
    for ( i=0;i<(items>>2); i++,*(unsigned int*)pw = p32, pw+=4);

    /* PART 3 : aligned on 32 bytes chunk, so we bypass cache lines */
    for(i=0;i<(size>>5);i++)
    {
        ALLOCO(pw);
        __asm__ volatile ("fmov %1,@%0"  : : "r"(pw),"d"(temp0) : "memory" );  pw+=8;
        __asm__ volatile ("fmov %1,@%0"  : : "r"(pw),"d"(temp0) : "memory" );  pw+=8;
        __asm__ volatile ("fmov %1,@%0"  : : "r"(pw),"d"(temp0) : "memory" );  pw+=8;
        __asm__ volatile ("fmov %1,@%0"  : : "r"(pw),"d"(temp0) : "memory" );  pw+=8;
    }
    PREFETCH(pw);
    __asm__ volatile("lds %0, fpscr" : : "r" (old_fpscr) ); // restore fpscr
    /* PART 4 : remaining data on 32 bytes word */
    for (i=0;i<(size &0x1C)>>2 ; i++,*(unsigned int*)pw=p32,   pw+=4 );
    /* PART 5 : unaligned bytes */
    for(i=0;i<(size&3);i++,*(unsigned char*)pw++ = p32);
}

/* ========================================================================
   Name:        doOp
   Description: execute the following signed code
   ======================================================================== */
void doOp(do_op_t *cmd)
{
    void *src;

    while( (cmd->op&0xFF) != OP_END )
    {
        /* If the source is NULL, use the value specified */
        src = cmd->src ? (void*)cmd->src : (void*)&cmd->val;

        switch(cmd->op & 0xFF)
        {
            case OP_WRITE_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_WRITE_U8\n\r");)
                if(cmd->val == 0)
                    **( U8**)cmd->dst =   ( U8  )cmd->src;
                else if(cmd->val == 1)
                    **( U8**)cmd->dst =  *( U8 *)cmd->src;
                else if(cmd->val == 2)
                    **( U8**)cmd->dst = **( U8**)cmd->src;
                break;
            case OP_WRITE_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_WRITE_U16\n\r");)
                if(cmd->val == 0)
                    **( U16**)cmd->dst =   ( U16  )cmd->src;
                else if(cmd->val == 1)
                    **( U16**)cmd->dst =  *( U16 *)cmd->src;
                else if(cmd->val == 2)
                    **( U16**)cmd->dst = **( U16**)cmd->src;
                break;
            case OP_WRITE_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_WRITE_U32\n\r");)
                if(cmd->val == 0)
                    **( U32**)cmd->dst =   ( U32  )cmd->src;
                else if(cmd->val == 1)
                    **( U32**)cmd->dst =  *( U32 *)cmd->src;
                else if(cmd->val == 2)
                    **( U32**)cmd->dst = **( U32**)cmd->src;
                break;

            case OP_READ_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_READ_U8\n\r");)
                *( U8*)cmd->dst = *( U8*)src;
                break;
            case OP_READ_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_READ_U16\n\r");)
                *(U16*)cmd->dst = *(U16*)src;
                break;
            case OP_READ_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_READ_U32\n\r");)
                *(U32*)cmd->dst = *(U32*)src;
                break;

            case OP_OR_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_OR_U8\n\r");)
                *( U8*)cmd->dst |= *( U8*)src;
                break;
            case OP_OR_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_OR_U16\n\r");)
                *(U16*)cmd->dst |= *(U16*)src;
                break;
            case OP_OR_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_OR_U32\n\r");)
                *(U32*)cmd->dst |= *(U32*)src;
                break;

            case OP_AND_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_AND_U8\n\r");)
                *( U8*)cmd->dst &= *( U8*)src;
                break;
            case OP_AND_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_AND_U16\n\r");)
                *(U16*)cmd->dst &= *(U16*)src;
                break;
            case OP_AND_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_AND_U32\n\r");)
                *(U32*)cmd->dst &= *(U32*)src;
                break;

            case OP_ADD_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_ADD_U8\n\r");)
                *( U8*)cmd->dst += *( U8*)src;
                break;
            case OP_ADD_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_ADD_U16\n\r");)
                *(U16*)cmd->dst += *(U16*)src;
                break;
            case OP_ADD_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_ADD_U32\n\r");)
                *(U32*)cmd->dst += *(U32*)src;
                break;

            case OP_SUB_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_SUB_U8\n\r");)
                *( U8*)cmd->dst -= *( U8*)src;
                break;
            case OP_SUB_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_SUB_U16\n\r");)
                *(U16*)cmd->dst -= *(U16*)src;
                break;
            case OP_SUB_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_SUB_U32\n\r");)
                *(U32*)cmd->dst -= *(U32*)src;
                break;

            case OP_IF_U8:
                DEBUG_OP(uart_send_string("OP CODE : OP_IF_U8\n\r");)
                switch(cmd->op & 0xFF00)
                {
                    case IF_HIGHER:
                        if(*( U8*)src < *( U8*)cmd->dst)
                            cmd++;
                        break;
                    case IF_SMALLER:
                        if(*( U8*)src > *( U8*)cmd->dst)
                            cmd++;
                        break;
                    case OP_EQUALS:
                        if(*( U8*)src != *( U8*)cmd->dst)
                            cmd++;
                        break;
                    case OP_DIFFERENT:
                        if(*( U8*)src == *( U8*)cmd->dst)
                            cmd++;
                        break;

                    default:
                        break;
                }
                break;
            case OP_IF_U16:
                DEBUG_OP(uart_send_string("OP CODE : OP_IF_U16\n\r");)
                switch(cmd->op & 0xFF00)
                {
                    case IF_HIGHER:
                        if(*(U16*)src < *(U16*)cmd->dst)
                            cmd++;
                        break;
                    case IF_SMALLER:
                        if(*(U16*)src > *(U16*)cmd->dst)
                            cmd++;
                        break;
                    case OP_EQUALS:
                        if(*(U16*)src != *(U16*)cmd->dst)
                            cmd++;
                        break;
                    case OP_DIFFERENT:
                        if(*(U16*)src == *(U16*)cmd->dst)
                            cmd++;
                        break;

                    default:
                        break;
                }
                break;
            case OP_IF_U32:
                DEBUG_OP(uart_send_string("OP CODE : OP_IF_U32\n\r");)
                switch(cmd->op & 0xFF00)
                {
                    case IF_HIGHER:
                        if(*(U32*)src < *(U32*)cmd->dst)
                            cmd++;
                        break;
                    case IF_SMALLER:
                        if(*(U32*)src > *(U32*)cmd->dst)
                            cmd++;
                        break;
                    case OP_EQUALS:
                        if(*(U32*)src != *(U32*)cmd->dst)
                            cmd++;
                        break;
                    case OP_DIFFERENT:
                        if(*(U32*)src == *(U32*)cmd->dst)
                            cmd++;
                        break;

                    default:
                        break;
                }
                break;

            case OP_FORWARD:
                DEBUG_OP(uart_send_string("OP CODE : OP_FORWARD\n\r");)
                cmd += (cmd->val-1);
                break;
            case OP_BACKWARD:
                DEBUG_OP(uart_send_string("OP CODE : OP_BACKWARD\n\r");)
                cmd -= (cmd->val+1);
                break;

            default:
                DEBUG_OP(uart_send_string("OP CODE : Unknown Operator\n\r");)
                break;
        }
        cmd++;
    }
}

void checkEnv()
{
    char                     *ptr;
    inverto_unique_id_type_t *iuid;
    char                      tmp[256];

    iuid = (inverto_unique_id_type_t *) IUID_START;

    printf("\nATTENTION: Use the command 'setenv' to change the settings:\n\n");
    printf("Settings :\n");
    printf("    bootargs ARGS     - Set the bootargs\n");
    printf("    config PIO        - Override the PIO configuration\n");
    printf("    ethaddr MAC       - Override the MAC address stored in the NOR Flash\n"
           "                          MAC must be 17 characters long (STB must be restarted)\n");
    printf("    model MODEL       - Override the MODEL Number (to use the previous LOADER)\n"
           "                          MAC must be 4 characters long\n");
    printf("    hw HW             - Override the Hardware ID stored in the NOR Flash\n"
           "                          HW must be 32 characters long\n");
    printf("    sn SN             - Override the Serial Number stored in the NOR Flash\n"
           "                          SN must be 16 characters long\n");
    printf("    selfstart [0|1|2] - Force the bootloader to run the default test before booting\n"
           "                          0 -> Default Application\n"
           "                          1 -> UBOOT command\n"
           "                          2 -> NFS\n");
    printf("    selftest [0|1]    - Force the bootloader to run the default test before booting\n"
           "                          0 -> OFF\n"
           "                          1 -> ON\n");
    printf("\n");

    ptr = getenv("bootargs");
    printf("bootargs : %s\n           dflt %s\n", ptr, NULL);
    ptr = getenv("selfstart");
    printf("selfstart: %s\n           dflt %s\n", ptr, NULL);
    ptr = getenv("selftest");
    printf("selftest : %s\n           dflt %s\n", ptr, NULL);

    ptr = getenv("config");
    printf("config   : env  %s\n      dflt %X\n", ptr, board_getConfig(0));

    ptr = getenv("ethaddr");
    memset(tmp, 0, sizeof(tmp));
    setStrArg(tmp, NULL, (I8*)MAC_START, 17);
    printf("ethaddr  : env  %s\n           dflt %s\n", ptr, tmp);

    ptr = getenv("model");
    printf("model    : env  %s\n           dflt %X\n", ptr, MODEL_TYPE);

    ptr = setU32Arg(tmp, NULL, swap32(iuid->HardwareVersion1), NO_FLAG);
    ptr = setU32Arg(ptr, NULL, swap32(iuid->HardwareVersion2), NO_FLAG);
    ptr = setU32Arg(ptr, NULL, swap32(iuid->HardwareVersion3), NO_FLAG);
    ptr = setU32Arg(ptr, NULL, swap32(iuid->HardwareVersion4), NO_FLAG);
    ptr = getenv("hw");
    printf("hw       : env  %s\n           dflt %s\n", ptr, tmp);

    ptr = getenv("sn");
    memset(tmp, 0, sizeof(tmp));
    setStrArg(tmp, NULL, iuid->SerialNumber, SN_SIZE);
    printf("sn       : env  %s\n           dflt %s\n", ptr, tmp);
}

void tryOp(U32 op, void *src, void *dst, U32 val)
{
	do_op_t cmd[3];
	memset(cmd, 0, sizeof(cmd));

	cmd[0].op  = op;
	cmd[0].src = src;
	cmd[0].dst = dst;
	cmd[0].val = val;

	doOp(cmd);
}


