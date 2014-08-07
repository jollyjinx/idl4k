#include "iboot.h"
#include "asm/sh4reg.h"

void doOp(do_op_t *cmd);

#define DEBUG_MSG(x)

const U8 conv[16] = "0123456789ABCDEF";

static int performAction(extraAction_t *extraAction)
{
    int error;
    UL32 unzipped_size = -1;

    error = false;
    switch(extraAction->action)
    {
        case ACTION_COPY :
            DEBUG_MSG(printf("INFO : ACTION_COPY\r\n");)
            memcpyf((void*)extraAction->dst, (void*)extraAction->src, extraAction->size);
            break;
        case ACTION_HW_ARGS :
            DEBUG_MSG(printf("INFO : ACTION_HW_ARGS\r\n");)
            addDynamicArgs((I8*)extraAction->dst);
            break;
        case ACTION_UNZIP :
            DEBUG_MSG(printf("INFO : ACTION_UNZIP\r\n");)
            if( uncompress((U8*)extraAction->dst, &unzipped_size, (U8*)extraAction->src, extraAction->size, 1) )
            {
                error = true;
            }
            break;
        case ACTION_SET :
            DEBUG_MSG(printf("INFO : ACTION_SET\r\n");)
            memset((void*)extraAction->dst, extraAction->src, extraAction->size);
            break;
        case ACTION_DOOP:
            DEBUG_MSG(printf("INFO : ACTION_DOOP\r\n");)
            doOp((do_op_t*)extraAction->data);
            break;
        default:
            DEBUG_MSG(printf("ERROR : extaAction->action is not properly defined\r\n");)
            error = true;
            break;
    }

    return error;
}

void launch_iboot_img(U32 addr)
{
    U32             j;
    U32             error;
    moduleHeader_t *header;

    /* We might need to load the V2F */
    if(addr == 0x80801000)
    {
        nand_burstInit();
        nand_burstRead( NAND_PART_0_START, MODULE_3_START, NAND_PART_0_STOP-NAND_PART_0_START );
        addr = MODULE_3_START;
    }

#if defined(CONFIG_USE_FTA_SSA)
    ssaCheck((unsigned char *)addr);
#endif

    header = (moduleHeader_t*)addr;

    if(header->pattern != MODULE_PATTERN)
        return;

//    printf("Found a valid I-Boot header\n");

    /******************************************************************************/
    /* For more stability, we flush the D-cache                                   */
    /******************************************************************************/
	sh_flush_cache_all();

    /******************************************************************************/
    /*  For more stability, we invalidate the data stored in the I-cache          */
    /******************************************************************************/
    *SH4_CCN_CCR = *SH4_CCN_CCR | 0x800;

    for(j=0;j<header->nbExtraAction;j++)
    {
        error = performAction(&header->extraAction[j]);
        if(error)
        {
            DEBUG_MSG(printf("ERROR : extaAction failed !\n");)
            return;
        }
    }

    termFtaLib();

    if(header->application == MODULE_3)
    {
//      printf("KERNEL (%qu)\n", get_ticks());
        bootm_linux((void*)0x80801000);
    }
    else if(header->application == MODULE_4)
    {
        bootm_linux((void*)0x80001000);
    }
    else
    {
        bootm_linux((void*)0x88000000);
    }
}
