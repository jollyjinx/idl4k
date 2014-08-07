/******************************************************************************/
/*                                                                            */
/* File name   : CONFIG.H                                                     */
/*                                                                            */
/* Description : Board specific definitions                                   */
/*                                                                            */
/* COPYRIGHT (C) FTA 2009                                                     */
/*                                                                            */
/* Date               Modification                 Name                       */
/* ----               ------------                 ----                       */
/* 10/05/09           Created                      E. FRADIN                  */
/*                                                                            */
/******************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

/* PIO definition */
/* ============== */
#define NB_PIO  10

/* UART */
#define PIO_UART_TX             1,0
#define PIO_UART_RX             1,1
/* TUNER */
#define PIO_FE_RESET            1,2
/* I2C */
#define PIO_I2C_SCLA            2,2
#define PIO_I2C_SCAA            2,3
#define PIO_I2C_SCLB            2,5
#define PIO_I2C_SCAB            2,6
/* LOGO & KEYS LED */
#define PIO_LED_LOGO            3,1
#define PIO_LED_KEYS            3,2
/* PERIPHERAL ENABLE */
#define PIO_PERIPH_EN           3,7
/* SCART */
#define PIO_SB_AV               4,1
#define PIO_SB_169              4,2
#define PIO_FB                  4,3
/* VFD */
#define PIO_FP_POWER            5,4
#define PIO_FP_DOWN             5,5
#define PIO_FP_UP               5,6
/* HDMI */
#define PIO_HDMI_HPD            9,7


/* CONFIG */
/* ====== */
#define PIO_CONFIG_PORT         6
#define PIO_CONFIG_MASK         0x0F
#define PIO_CONFIG_BMP_MASK     0x03
#define PIO_CONFIG_FP_MASK      0x04

/* I2C Bus */
/* ======= */
#define NUM_SSC                 2
#define HDMI_BUS                0
#define EEPROM_BUS              1
#define FE_BUS                  1

#define HDMI_I2C_ADDR           0xA0

#define EEPROM_ADDRESS          0xA0
#define EEPROM_SSD_REG          0x0480
#define EEPROM_SSD_REG_LEN      2

#define FE_ADDRESS              0xD0
#define FE_ID_REG               0xF100
#define FE_ID_REG_LEN           2


/* PERIPHERAL organization */
/* ======================= */
#define UART_REGS_BASE          ST40_ASC1_REGS_BASE


/* CLOCKS definition */
/* ================= */
#define P_CLOCK_RATE            66000000            /* clock rate for CSP */


/* I2C definition */
/* ============== */
#define CFG_I2C_1NS             19                  /* Empirical measurement for 1ns */
#define CFG_I2C_SPEED           100000

/* TIMEOUT */
/* ======= */
#define FLASH_TIMEOUT           4000000


/* VIDEO */
/* ======= */
#define VIDEO_WIDTH             720
#define VIDEO_HEIGHT            576


/* OTHER */
/* ===== */
#define MODEL_TYPE              0x3700
#define BOARD_ID                0x4B333549


/* ARCH */
/* ==== */
#define NOR_START                      0x00000000
#	define I_BOOT_START                0x00000000
#		define BEP_START               0x00000000
#		define BEP_STOP                0x000038E0
#		define DA1_START               0x000038E0
#		define DA1_STOP                0x00003AE0
#		define DA2_FACTORY_START       0x00003AE0
#		define DA2_FACTORY_STOP        0x00004020
#		define ST_BC_CHECK_SIZE_START  0x0000403C
#		define ST_BC_CHECK_SIZE_STOP   0x00004040
#		define SCS_FA_HOLE_A_START     0x00004040
#		define ST_SIGN_START           0x00004080
#		define ST_SIGN_STOP            0x00004380
#		define SCS_FA_HOLE_A_STOP      0x00004380
#		define BOOTCODE_START          0x00004380
#		define BOOTCODE_STOP           0x0001FC00
#		define SCS_FA_HOLE_B_START     0x0001FC00
#			define PK_START            0x0001FC00
#			define CSC_START           0x0001FD00
#			define IUID_START          0x0001FF58
#			define MAC_START           0x0001FF80
#			define E_GK_START          0x0001FF94
#			define C_GK_START          0x0001FFA4
#		define SCS_FA_HOLE_B_STOP      0x00020000
#	define I_BOOT_STOP                 0x00020000

#	define DA2_START                   0x00020000
#	define DA2_STOP                    0x00020540
#	define MODULE_1_START              0x00020540
#	define MODULE_1_STOP               0x00030000

#	define MODULE_2_START              0x00030000
#	define MODULE_2_STOP               0x00040000

#	define PRIVATE_KEYS_START          0x00040000
#	define PRIVATE_KEYS_STOP           0x00050000

#	define DA2_REDUNDANT_START         0x00050000
#	define DA2_REDUNDANT_STOP          0x00050540
#	define I_BOOT_IMAGE_STOP           0x00060000

#	define MODULE_4_START              0x002C0000
#	define MODULE_4_STOP               0x00800000
#define NOR_STOP                       0x00800000


#define NAND_START                     0x00000000
#	define NAND_PART_0_START           0x00000000
#	define NAND_PART_0_STOP            0x03000000
#	define NAND_PART_1_START           0x03000000
#	define NAND_PART_1__STOP           0x07FC0000
#	define NAND_BBT_START              0x07FC0000
#	define NAND_BBT_STOP               0x08000000
#define NAND_STOP                      0x08000000


/* RAM Map (Bootcode) */
#define RAM_START                      0x80000000
#	define SHADOW_VIDEO_DISPLAY_START  0x80000000
#	define MODULE_3_START              0x857FFFC0
#	define MODULE_3_STOP               0x88500000
#	define MAIN_VIDEO_NODE_START       0x88500000
#	define MAIN_VIDEO_NODE_STOP        0x88504000
#	define MAIN_VIDEO_DISPLAY_START    0x88504000
#define RAM_STOP                       0x90000000

#define NAND_CHIP_SIZE                 (NAND_STOP-NAND_START)

#endif  /* __CONFIG_H__ */
