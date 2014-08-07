/*
 * Synopsis : Error Correction Codes (ECC) Algorithms.
 *
 * Copyright (c) 2008-2011 STMicroelectronics Limited.  All right reserved.
 *
 * See ecc.h for a description of this module.
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

#include <common.h>

#if defined(CONFIG_CMD_NAND)

#include <asm/ecc.h>


const unsigned char  ecc_bit_count_table[256] =   /* Parity look up table */
{
  0, 1, 1, 2, 1, 2, 2, 3,
  1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7,
  5, 6, 6, 7, 6, 7, 7, 8
};


#if defined(CFG_NAND_ECC_HW3_128) || defined(CFG_NAND_ECC_AFM4)

static const unsigned char byte_parity_table[] =   /* Parity look up table */
{
  0x00, 0x2B, 0x2D, 0x06, 0x33, 0x18, 0x1E, 0x35,
  0x35, 0x1E, 0x18, 0x33, 0x06, 0x2D, 0x2B, 0x00,
  0x4B, 0x60, 0x66, 0x4D, 0x78, 0x53, 0x55, 0x7E,
  0x7E, 0x55, 0x53, 0x78, 0x4D, 0x66, 0x60, 0x4B,
  0x4D, 0x66, 0x60, 0x4B, 0x7E, 0x55, 0x53, 0x78,
  0x78, 0x53, 0x55, 0x7E, 0x4B, 0x60, 0x66, 0x4D,
  0x06, 0x2D, 0x2B, 0x00, 0x35, 0x1E, 0x18, 0x33,
  0x33, 0x18, 0x1E, 0x35, 0x00, 0x2B, 0x2D, 0x06,
  0x53, 0x78, 0x7E, 0x55, 0x60, 0x4B, 0x4D, 0x66,
  0x66, 0x4D, 0x4B, 0x60, 0x55, 0x7E, 0x78, 0x53,
  0x18, 0x33, 0x35, 0x1E, 0x2B, 0x00, 0x06, 0x2D,
  0x2D, 0x06, 0x00, 0x2B, 0x1E, 0x35, 0x33, 0x18,
  0x1E, 0x35, 0x33, 0x18, 0x2D, 0x06, 0x00, 0x2B,
  0x2B, 0x00, 0x06, 0x2D, 0x18, 0x33, 0x35, 0x1E,
  0x55, 0x7E, 0x78, 0x53, 0x66, 0x4D, 0x4B, 0x60,
  0x60, 0x4B, 0x4D, 0x66, 0x53, 0x78, 0x7E, 0x55,
  0x55, 0x7E, 0x78, 0x53, 0x66, 0x4D, 0x4B, 0x60,
  0x60, 0x4B, 0x4D, 0x66, 0x53, 0x78, 0x7E, 0x55,
  0x1E, 0x35, 0x33, 0x18, 0x2D, 0x06, 0x00, 0x2B,
  0x2B, 0x00, 0x06, 0x2D, 0x18, 0x33, 0x35, 0x1E,
  0x18, 0x33, 0x35, 0x1E, 0x2B, 0x00, 0x06, 0x2D,
  0x2D, 0x06, 0x00, 0x2B, 0x1E, 0x35, 0x33, 0x18,
  0x53, 0x78, 0x7E, 0x55, 0x60, 0x4B, 0x4D, 0x66,
  0x66, 0x4D, 0x4B, 0x60, 0x55, 0x7E, 0x78, 0x53,
  0x06, 0x2D, 0x2B, 0x00, 0x35, 0x1E, 0x18, 0x33,
  0x33, 0x18, 0x1E, 0x35, 0x00, 0x2B, 0x2D, 0x06,
  0x4D, 0x66, 0x60, 0x4B, 0x7E, 0x55, 0x53, 0x78,
  0x78, 0x53, 0x55, 0x7E, 0x4B, 0x60, 0x66, 0x4D,
  0x4B, 0x60, 0x66, 0x4D, 0x78, 0x53, 0x55, 0x7E,
  0x7E, 0x55, 0x53, 0x78, 0x4D, 0x66, 0x60, 0x4B,
  0x00, 0x2B, 0x2D, 0x06, 0x33, 0x18, 0x1E, 0x35,
  0x35, 0x1E, 0x18, 0x33, 0x06, 0x2D, 0x2B, 0x00
};

/*******************************************************************************/
#define COL_LOOP_STEP(c__f, c__e, c__o, c__t) \
  c__o ^= (c__f ? c__t : 1); \
  c__e ^= (c__f ? 1 : c__t);

/* Generate 3 byte ECC code for ecc_size block p_data.
   "p_data" is a pointer to the data and must be 4-byte aligned.
   "size" gives length of "p_data" - one of enum ecc_size.
 */
ecc_t ecc_gen(const unsigned char* p_data, const enum ecc_size size)
{
  const unsigned long* const p_data_long = (unsigned long*)p_data;
  unsigned long parity_bits[18];  /* maximum number */
  unsigned long reg32;
  unsigned long temp;
  unsigned long int_cnt;
  unsigned long bit_cnt;

  unsigned int num_parity_bits;

  unsigned char* p_byt;
  unsigned char byte_reg;
  unsigned char byte_a;
  unsigned char byte_b;
  unsigned char byte_c;
  unsigned char byte_d;

  ecc_t result;

  switch (size)
  {
    case ECC_128:
    default:
      num_parity_bits = 14;
      break;
    case ECC_256:
      num_parity_bits = 16;
      break;
    case ECC_512:
      num_parity_bits = 18;
      break;
  }

  /* Initialize variables */
  byte_reg = 0;
  reg32 = 0;

  result.byte[0] = result.byte[1] = result.byte[2] = 0;

  for(bit_cnt = 0; bit_cnt < num_parity_bits; bit_cnt ++)
  {
    parity_bits[bit_cnt] = 0;
  } /* for bit_cnt */

  /* Build up column parity */
  for(int_cnt = 0; int_cnt < size/sizeof(unsigned long); int_cnt++)
  {
    temp = p_data_long[int_cnt];

    switch (size)
    {
      case ECC_512:
        COL_LOOP_STEP((int_cnt & 0x40), parity_bits[16], parity_bits[17], temp);
        /* fall through */
      case ECC_256:
        COL_LOOP_STEP((int_cnt & 0x20), parity_bits[14], parity_bits[15], temp);
        /* fall through */
      case ECC_128:
        COL_LOOP_STEP((int_cnt & 0x01), parity_bits[4], parity_bits[5], temp);
        COL_LOOP_STEP((int_cnt & 0x02), parity_bits[6], parity_bits[7], temp);
        COL_LOOP_STEP((int_cnt & 0x04), parity_bits[8], parity_bits[9], temp);
        COL_LOOP_STEP((int_cnt & 0x08), parity_bits[10], parity_bits[11], temp);
        COL_LOOP_STEP((int_cnt & 0x10), parity_bits[12], parity_bits[13], temp);
    }
  } /* for int_cnt */

  reg32 = parity_bits[12] ^ parity_bits[13];

  p_byt = (unsigned char*)&reg32;
#if __LITTLE_ENDIAN__
  byte_a = p_byt[3];
  byte_b = p_byt[2];
  byte_c = p_byt[1];
  byte_d = p_byt[0];
#else
  byte_a = p_byt[0];
  byte_b = p_byt[1];
  byte_c = p_byt[2];
  byte_d = p_byt[3];
#endif

  byte_reg = byte_a ^ byte_b ^ byte_c ^ byte_d;

  byte_reg = byte_parity_table[byte_reg] >> 1;


  /* Create line parity */
  parity_bits[0] = byte_d ^ byte_b;
  parity_bits[1] = byte_c ^ byte_a;
  parity_bits[2] = byte_d ^ byte_c;
  parity_bits[3] = byte_b ^ byte_a;

  for(bit_cnt = 4; bit_cnt < num_parity_bits; bit_cnt++)
  {
    p_byt = (unsigned char*)(parity_bits + bit_cnt);
    p_byt[0] ^= (p_byt[1] ^ p_byt[2] ^ p_byt[3]); /* NB Only LS Byte of parity_bits used from now on */
  } /* for bit_cnt */

  /* Calculate final ECC code */
  for(bit_cnt = 0; bit_cnt < 8; bit_cnt ++)
    result.byte[0] |= (byte_parity_table[ (unsigned char)parity_bits[bit_cnt] ] & 0x01) << bit_cnt;
  for(; bit_cnt < 16 && bit_cnt < num_parity_bits; bit_cnt ++)
    result.byte[1] |= (byte_parity_table[ (unsigned char)parity_bits[bit_cnt] ] & 0x01) << (bit_cnt - 8);
  for(; bit_cnt < num_parity_bits; bit_cnt ++)
    result.byte[2] |= (byte_parity_table[ (unsigned char)parity_bits[bit_cnt] ] & 0x01) << (bit_cnt - 16);

  result.byte[2] = (unsigned char)(byte_reg << 2) | (result.byte[2] & 0x03);

  return result;
} /* ecc_gen */


/*******************************************************************************/
/* Detect and correct a 1 bit error in a 128, 256 or 512 byte block.
   "p_data" is a pointer to the data.
   "old_ecc" is the proper ECC for the data.
   "new_ecc" is the ECC generated from the (possibly) corrupted data.
   The size of the block is given in "size".

   Returns whether the data needed correcting, or was not correctable.
   If the result code is E_D1_CHK, then the data will have been modified.
 */
enum ecc_check ecc_correct(unsigned char* p_data,
                           ecc_t old_ecc,
                           ecc_t new_ecc,
                           enum ecc_size size)
{
  unsigned char bit_cnt02;
  unsigned char bit_addr02;
  unsigned int byte_addr02;

  unsigned char ecc_xor[3];

  unsigned char error_bit_count;
  switch (size)
  {
    case ECC_128:
    default:
      error_bit_count = 10;
      break;
    case ECC_256:
      error_bit_count = 11;
      break;
    case ECC_512:
      error_bit_count = 12;
      break;
  }

  /* Basic Error Detection phase */
  ecc_xor[0] = new_ecc.byte[0] ^ old_ecc.byte[0];
  ecc_xor[1] = new_ecc.byte[1] ^ old_ecc.byte[1];
  ecc_xor[2] = new_ecc.byte[2] ^ old_ecc.byte[2];

  if ((ecc_xor[0] | ecc_xor[1] | ecc_xor[2]) == 0)
  {
    return E_NO_CHK;  /* No errors */
  }
  /* If we get here then there were errors */

  if (size == ECC_512)
  {
    /* 512-byte error correction requires a little more than 128 or 256.
       If there is a correctable error then the xor will have 12 bits set,
       but there can also be 12 bits set in some uncorrectable errors.
       This can be solved by xoring the odd and even numbered bits.

       0xAA = 10101010
       0x55 = 01010101
     */
    bit_cnt02  = ecc_bit_count_table[ ((ecc_xor[0] & 0xAA) >> 1) ^ (ecc_xor[0] & 0x55) ];
    bit_cnt02 += ecc_bit_count_table[ ((ecc_xor[1] & 0xAA) >> 1) ^ (ecc_xor[1] & 0x55) ];
    bit_cnt02 += ecc_bit_count_table[ ((ecc_xor[2] & 0xAA) >> 1) ^ (ecc_xor[2] & 0x55) ];
  }
  else
  {
    /* Counts the number of bits set in ecc code */
    bit_cnt02  = ecc_bit_count_table[ ecc_xor[0] ];
    bit_cnt02 += ecc_bit_count_table[ ecc_xor[1] ];
    bit_cnt02 += ecc_bit_count_table[ ecc_xor[2] ];
  }

  if (bit_cnt02 == error_bit_count)
  {
    /* Set the bit address */
    bit_addr02 = ((ecc_xor[2] >> 3) & 0x01) |
                 ((ecc_xor[2] >> 4) & 0x02) |
                 ((ecc_xor[2] >> 5) & 0x04);

    /* Evaluate 2 LS bits of address */
    byte_addr02 = ((ecc_xor[0] >> 1) & 0x01) | ((ecc_xor[0] >> 2) & 0x02);

    /* Add in remaining bits of address */
    switch (size)
    {
      case ECC_512:
        byte_addr02 |= (((unsigned int)ecc_xor[2]) << 7) & 0x100;
        /* Fall through */
      case ECC_256:
        byte_addr02 |= (ecc_xor[1] & 0x80);
        /* Fall through */
      case ECC_128:
        byte_addr02 |= ((ecc_xor[0] >> 3) & 0x04) |
                       ((ecc_xor[0] >> 4) & 0x08) |
                       ((ecc_xor[1] << 3) & 0x10) |
                       ((ecc_xor[1] << 2) & 0x20) |
                       ((ecc_xor[1] << 1) & 0x40);
    }


    /* Correct bit error in the data */
    p_data[byte_addr02] ^= (0x01 << bit_addr02);

    /* NB p_old_code is okay, p_new_code is corrupt */

    return E_D1_CHK;  /* Data had 1-bit error (now corrected) */
  } /* if (bit_cnt02 == 11) */
  else
  {
    if (bit_cnt02 == 1)
    {
      return E_C1_CHK;  /* ECC code has 1-bit error, data is okay */
    } /* if (bit_cnt02 == 1) */
    else
    {
      return E_UN_CHK;  /* Uncorrectable Error */
    } /* else !(bit_cnt02 == 1) */
  } /* else !(bit_cnt02 == 11) */
} /* ecc_correct */
/*******************************************************************************/


#ifdef TESTING
/* To test this code, compile as follows:

       sh4gcc -mboard=<board> -g ecc.c -DTESTING

   This provides a sanity test ONLY. It gives complete coverage of the code,
   but does not fully exercise the mathmatics behind the algorithm.
 */
#include <stdlib.h>
#include <time.h>

int main()
{
  unsigned char data[512];
  ecc_t ecc1;
  ecc_t ecc2;
  int i, size;

  for (size = 128; size <= 512; size *= 2)
  {
    /* Create a data set */
    for (i = 0; i < size; i++)
      data[i] = i & 0xFF;

    /* Create the initial ecc */
    ecc1 = ecc_gen(data, size);

    /* Make sure that it doesn't try to "correct" it before it's corrupted */
    if (ecc_correct(data, ecc1, ecc1, size) != E_NO_CHK)
    {
      printf("Error: failed to detect good data.\n");
      exit(1);
    }

    for (i = 0; i < size; i++)
      if (data[i] != (i & 0xFF))
      {
  printf("Error: corrupted good data.\n");
  exit(1);
      }

    /* Deliberately corrupt the data with 1 bit error only */
    data[size-42] ^= 1;

    /* Create a new ecc for the bad data */
    ecc2 = ecc_gen(data, size);

    /* Make sure that it can fix the issues */
    if (ecc_correct(data, ecc1, ecc2, size) != E_D1_CHK)
    {
      printf("Error: failed to diagnose 1 bit data error.\n");
      exit(1);
    }

    for (i = 0; i < size; i++)
      if (data[i] != (i & 0xFF))
      {
        printf("Error: did not correct bad data.\n");
        exit(1);
      }
    /* Data is now GOOD */

    /* Deliberately corrupt the ECC with 1 bit error */
    ecc2.byte[0] = ecc1.byte[0];
    ecc2.byte[1] = ecc1.byte[1] ^ 4;
    ecc2.byte[2] = ecc1.byte[2];

    /* Simulate reading bad ECC from flash, but calculating good ECC from data */
    if (ecc_correct(data, ecc2, ecc1, size) != E_C1_CHK)
    {
      printf("Error: failed to diagnose 1 bit ECC error.\n");
      exit(1);
    }

    /* Deliberately corrupt the data with a 2-bit error */
    data[size-42] ^= 3;

    /* Create a new ecc for the bad data */
    ecc2 = ecc_gen(data, size);

    /* Check that it reports that it cannot correct the issue */
    if (ecc_correct(data, ecc1, ecc2, size) != E_UN_CHK)
    {
      printf("Error: failed to diagnose 2-bit corruption.\n");
      exit(1);
    }

    printf("ECC %d working as expected\n", size);
  }

  /* The above tests show that all is well with the various code paths.
     However, it does not test detection of 2-bit error detection so well.
     These tests generate and corrupt random data sets and ensures that
     the algorithm does not mis-diagnose the condition.

     Notes:
     1. There is nothing to stop it corrupting the same bit twice - in which
     case it will correctly diagnose a 1-bit error.
     2. 3-bit errors and worse cause undefined behaviour, so we don't test those.
   */
  printf("\nECC 512 random data tests (interrupt when satisfied)\n");

  srand(time(NULL));
  while (1)
  {
    for (i = 0; i < 512; i++)
      /* Random number 0..255 */
      data[i] = (unsigned char)(256.0 * rand() / (RAND_MAX + 1.0));

    ecc1 = ecc_gen(data, ECC_512);

    for (i = 0; i < 2; i++)
    {
      /* Random number 0..514 */
      int corruptbyte = (int)(515.0 * rand() / (RAND_MAX + 1.0));
      /* Random number 0..7 */
      int corruptbit = (int)(8.0 * rand() / (RAND_MAX + 1.0));

      if (corruptbyte < 512)
        data[corruptbyte] ^= 1 << corruptbit;
      else
        ecc1.byte[corruptbyte - 512] ^= 1 << corruptbit;

      printf("%3d:%d ", corruptbyte, corruptbit);
    }

    ecc2 = ecc_gen(data, ECC_512);

    if (ecc_correct(data, ecc1, ecc2, ECC_512) != E_UN_CHK)
      printf("Error: failed to diagnose 2-bit-corruption\n");
    else
      printf("OK\n");
  }

  return 0;
}
#endif	/* TESTING */
#endif	/* CFG_NAND_ECC_HW3_128 || CFG_NAND_ECC_AFM4 */
#endif	/* CONFIG_CMD_NAND */
