/*
 * "memcpy" implementation of SuperH
 *
 * Copyright (C) 1999  Niibe Yutaka
 * Copyright (c) 2002  STMicroelectronics Ltd
 *   Modified from memcpy.S and micro-optimised for SH4
 *   Stuart Menefy (stuart.menefy@st.com)
 *
 */
#include <linux/linkage.h>


!#define  MEMCPY_USES_FPU   ! to be defined if you want the FPU : faster for big 8 bytes aligned moves.
! #define  PRECEDING        ! to have the memcpy defined at top of file

! next line needs to be defined if you want the CACHE pre-allocation scheme
#if 1
    #define CACHE   movca.l
#else
    #define CACHE   mov.l
#endif
! next line needs to be defined if you want the prefetching
#if 1
    #define PREF(x) pref x
#else
    #define PREF(x)
#endif

#if defined(PRECEDING)
.align 5
ENTRY(memcpy)
ENTRY(memcpyf)                    ! r4 = dst , r5 = src, r6 = size
    mov     r4,r7           ! 00
    mov     #0x7F,r3        ! 01
    mov     r5,r0           ! 02 r0 = src
    cmp/hi  r3,r6           ! 03 if less than 63 bytes
    or      r4,r0           ! 04 r0 = src | dst
    bf/s    .Llastword128   ! 05 less  than r3 unaligned small byte copy
    tst     #3,r0           ! 06 if ((src|dst)&3)==0
    mov.l   r4,@-r15        ! 07 save dst into stack
    neg     r4,r0           ! 08
    shlr    r0              ! 09
    bf/s    1f              ! 10
    shlr    r0              ! 11
    mov.b   @r5+,r1         ! 12
    add     #-1 ,r6         ! 13
    mov.b   r1  ,@r4        ! 14
    add     #1  ,r4         ! 15
1:  bf      2f              ! 16
    mov.b   @r5+,r1         ! 17
    mov.b   @r5+,r0         ! 18
    add     #-2,r6          ! 19
    mov.b   r1  ,@r4        ! 20
    mov.b   r0  ,@(1,r4)    ! 21
    add     #2,r4           ! 22
2:  mov     #0x1c,r3        ! 23
    mov      r5,r0          ! 24
    neg      r4,r1          ! 25
    tst      #3,r0          ! 26
    and      r1,r3          ! 27 r3 = num words to move
    mov      r6,r7          ! 28
    shlr2    r3             ! 29 r3 = num words to move to be 32 bytes aligned 0,1,2,3,4,5,6,7
    shlr2    r7             ! 30
    bt/s    .Lbigmove32     ! 31 dst&3=0 -> aligned : word copy 4 by 4
    tst      r3,r3          ! 32
    tst      #1,r0          ! 33
    bt/s     .Lbigmove16    ! 34 dst&1=0  -> short aligned : word copy 2-2
    tst     r3  , r3        ! 35
    mov.l   r8 ,@-r15       ! 36
#endif

.align 5
.Lbigmove8 :                !   r1 ar, r2, al r4 = dst , r5 = src, r6 = size, r7= numwords
    mov.l   r9 ,@-r15       ! 000
    mov     #-4 , r0        ! 001
    mov     r5  , r1        ! 002   1 : offset from src
    mov     #3  , r9        ! 003
    and     r0  , r5        ! 004   src = src  ~3
    and     r9  , r1        ! 005   r1 = src&3
    mov.l   @r5+, r8        ! 006
    shld    r9  , r1        ! 007   r1 = ar<<3 = ar   ( -8 or -24 )
    mov     #32 , r2        ! 008
    neg     r1  , r1        ! 009
    bt/s    3f              ! 010   if r3 == 0, jump to aligned copy
    add     r1  , r2        ! 011   r2 = 32 - r1 = al ( 24 or 8 )
    sub     r3  ,r7         ! 012
2:  mov.l   @r5+,r9         ! 013
    dt      r3              ! 014
    shld    r1  ,r8         ! 015    = tmp0 >>ar
    mov     r9  ,r0         ! 016    =
    shld    r2  ,r0         ! 017    = tmp1 <<al
    or      r8  ,r0         ! 018    = (tmp0 >>ar) | (tmp1<<ar)
    mov.l   r0  ,@r4        ! 019
    add     #4  ,r4         ! 020   src and dst are aligned on 8 bytes
    bf/s    2b              ! 021
    mov     r9  ,r8         ! 022   tmp0 = tmp1
3:  mov     r7  ,r3         ! 023   have by chunk of 32 bytes = r7 is number of words
    shlr    r3              ! 024
    shlr2   r3              ! 025
    add     #-4 ,r3         ! 026  r2 = word/8 - 4 ,number of chunks we can move with prefetchs
    cmp/pl  r3              ! 027  if !(r2>0)
    bf      5f              ! 028  go to loop move if less than 128 bytes to move
    mov     #7 ,r9          ! 029
    mov.l   r14 ,@-r15      ! 030
    mov     r5  ,r14        ! 031
    mov.l   r12 ,@-r15      ! 032
    add     #127,r14        ! 033
    mov.l   r11 ,@-r15      ! 034
    and     r9 ,r7          ! 035 we need to compute the resulting number of words to move without prefetching
    mov.l   r10 ,@-r15      ! 036
    add     #32,r7          ! 037
4:  mov.l   @r5+,r9         ! 038 +
    dt      r3              ! 039 +
    mov.l   @r5+,r10        ! 040 +
    mov.l   @r5+,r11        ! 041 +
    mov.l   @r5+,r12        ! 042 +
    PREF   (@r14)           ! 043 +
    shld    r1 ,r8          ! 044 +
    mov     r9 ,r0          ! 045 +
    shld    r2 ,r0          ! 046 +
    or      r8 ,r0          ! 047 +
    CACHE   r0,@r4          ! 048 +
    shld    r1 ,r9          ! 049 +
    mov     r10,r8          ! 050 +
    shld    r2 ,r8          ! 051 +
    or      r9 ,r8          ! 052 +
    mov.l   r8 ,@(4,r4)     ! 053 +
    shld    r1 ,r10         ! 054 +
    mov     r11,r9          ! 055 +
    shld    r2 ,r9          ! 056 +
    or      r10,r9          ! 057 +
    mov.l   r9 ,@(8,r4)     ! 058 +
    shld    r1 ,r11         ! 059 +
    mov     r12,r10         ! 060 +
    shld    r2 ,r10         ! 061 +
    or      r11,r10         ! 062 +
    mov.l   r10,@(12,r4)    ! 063 +
    mov.l   @r5+,r9         ! 064 +
    mov.l   @r5+,r10        ! 065 +
    mov.l   @r5+,r11        ! 066 +
    mov.l   @r5+,r8         ! 067 +
    shld    r1 ,r12         ! 068 +
    mov     r9 ,r0          ! 069 +
    shld    r2 ,r0          ! 070 +
    or      r12,r0          ! 071 +
    mov.l   r0 ,@(16,r4)    ! 072 +
    shld    r1 ,r9          ! 073 +
    mov     r10,r12         ! 074 +
    shld    r2 ,r12         ! 075 +
    or      r9 ,r12         ! 076 +
    mov.l   r12,@(20,r4)    ! 077 +
    shld    r1 ,r10         ! 078 +
    mov     r11,r9          ! 079 +
    shld    r2 ,r9          ! 080 +
    or      r10,r9          ! 081 +
    mov.l   r9 ,@(24,r4)    ! 082 +
    shld    r1 ,r11         ! 083 +
    mov     r8 ,r10         ! 084 +
    shld    r2 ,r10         ! 085 +
    or      r11,r10         ! 086 +
    mov.l   r10,@(28,r4)    ! 087 +
    add     #32,r14         ! 088 +
    bf/s    4b              ! 089 +
    add     #32 ,r4         ! 090 +
    mov.l   @r15+,r10       ! 091
    mov.l   @r15+,r11       ! 092
    mov.l   @r15+,r12       ! 093
    mov.l   @r15+,r14       ! 094
5:  tst     r7, r7          ! 095 r7 remaining words, number from 0 to 31
    bt      7f              ! 096
6:  mov.l   @r5+,r9         ! 097
    dt      r7              ! 098
    shld    r1  ,r8         ! 099       r8 = tmp0 >>ar
    mov     r9  ,r0         ! 100       r9 =
    shld    r2  ,r0         ! 101       r0 = tmp1 <<al
    or      r8  ,r0         ! 102       r8 = (tmp0 >>ar) | (tmp1<<ar)
    mov.l   r0  ,@r4        ! 103
    add     #4  ,r4         ! 104       src and dst are aligned on 8 bytes
    bf/s    6b              ! 105
    mov     r9  ,r8         ! 106       tmp0 = tmp1
7:  mov     r6  ,r0         ! 107       r0 = remaining last bytes
    tst     #3  ,r0         ! 108
    neg     r1  ,r1         ! 109
    mov     #-3 ,r3         ! 110
    add     #-4 ,r5         ! 111
    shld    r3  ,r1         ! 112
    add     r1  ,r5         ! 113   just R5 to be byte aligned
    bt/s    9f              ! 114
    and     #3  ,r0         ! 115
8:  mov.b   @r5+,r1         ! 116
    dt      r0              ! 117
    mov.b   r1  ,@r4        ! 118
    bf/s    8b              ! 119
    add     #1  ,r4         ! 120
9:  mov.l   @r15+,r9        ! 121
    mov.l   @r15+,r8        ! 122
    rts                     ! 123
    mov.l   @r15+,r0        ! 124

.align 5
.Llastword128:              ! move up to 63 bytes in dichotomic checking , src and dst are aligned on 4 bytes.
    bf     .Llastbytes124   ! 00 if less than 63, but src and dst are not aligned on 4 bytes -> do byte per byte
    mov     #3,r3           ! 01
    and     r6,r3           ! 02
    shlr2   r6              ! 03
    shlr    r6              ! 04
    bf/s    1f              ! 05
    shlr    r6              ! 06
    mov.l   @r5+,r0         ! 07
    mov.l   r0  ,@r4        ! 08
    add     #4  ,r4         ! 09
1:  bf/s    2f              ! 10
    shlr    r6              ! 11
    mov.l   @r5+,r0         ! 12
    mov.l   @r5+,r1         ! 13
    mov.l   r0 , @r4        ! 14
    mov.l   r1 , @(4,r4)    ! 15
    add     #8 ,r4          ! 16
2:  bf/s    4f              ! 17
    tst     r6,r6           ! 18
3:  mov.l   @r5+,r0         ! 19
    mov.l   @r5+,r1         ! 20
    mov.l   @r5+,r2         ! 21
    mov.l   r0,@(0,r4)      ! 22
    mov.l   @r5+,r0         ! 23
    mov.l   r1,@(4,r4)      ! 24
    mov.l   r2,@(8,r4)      ! 25
    mov.l   r0,@(12,r4)     ! 26
    add     #16,r4          ! 27
4:  bt      6f              ! 28
#ifdef MEMCPY_USES_FPU
5:  mov.l   @r5+,r0         ! 29
    mov.l   @r5+,r1         ! 30
    mov.l   @r5+,r2         ! 31
    fmov.s  @r5+,fr0        ! 32
    fmov.s  @r5+,fr1        ! 33
    fmov.s  @r5+,fr2        ! 34
    fmov.s  @r5+,fr3        ! 35
    fmov.s  @r5+,fr4        ! 36
    dt      r6              ! 37
    mov.l   r0,@(0,r4)      ! 38
    mov.l   r1,@(4,r4)      ! 39
    mov.l   r2,@(8,r4)      ! 40
    add     #32,r4          ! 41
    fmov.s  fr4,@-r4        ! 42
    fmov.s  fr3,@-r4        ! 43
    fmov.s  fr2,@-r4        ! 44
    fmov.s  fr1,@-r4        ! 45
    fmov.s  fr0,@-r4        ! 46
    bf/s    5b              ! 47
    add     #20,r4          ! 48
#else
5:  mov.l   @r5+,r0         ! 29
    mov.l   @r5+,r1         ! 30
    mov.l   @r5+,r2         ! 31
    dt      r6              ! 32
    mov.l   r0,@(0,r4)      ! 33
    mov.l   r1,@(4,r4)      ! 34
    mov.l   r2,@(8,r4)      ! 35
    mov.l   @r5+,r0         ! 36
    mov.l   @r5+,r1         ! 37
    mov.l   @r5+,r2         ! 38
    mov.l   r0,@(12,r4)     ! 39
    mov.l   r1,@(16,r4)     ! 40
    mov.l   r2,@(20,r4)     ! 41
    mov.l   @r5+,r0         ! 42
    mov.l   @r5+,r1         ! 43
    mov.l   r0,@(24,r4)     ! 44
    mov.l   r1,@(28,r4)     ! 45
    bf/s    5b              ! 46
    add     #32,r4          ! 47
#endif
6:  shlr    r3              ! 49
    bf/s    7f              ! 50
    shlr    r3              ! 51
    mov.b   @r5+,r0         ! 52
    mov.b   r0,@r4          ! 53
    add     #1,r4           ! 54
7:  bf      8f              ! 55
    mov.b   @r5+,r1         ! 56
    mov.b   @r5+,r0         ! 57
    mov.b   r1,@r4          ! 58
    mov.b   r0,@(1,r4)      ! 59
8:  rts                     ! 60
    mov     r7,r0           ! 61    return dst as result
.Llastbytes124:             ! 1-2-4 : 25 cycles + 3.13 cycles\bytes
    shlr    r6              ! 00
    bf/s    1f              ! 01
    shlr    r6              ! 02
    mov.b   @r5+,r0         ! 03
    mov.b   r0  ,@r4        ! 04
    add     #1  ,r4         ! 05
1:  bf/s    2f              ! 06
    tst     r6  ,r6         ! 07
    mov.b   @r5+,r1         ! 08
    mov.b   @r5+,r0         ! 09
    mov.b   r1  ,@r4        ! 10
    mov.b   r0  ,@(1,r4)    ! 11
    add     #2  ,r4         ! 12
2:  bt/s    4f              ! 13
    add     #-4,r4          ! 14
3:  mov.b   @r5+,r0         ! 15 +
    mov.b   @r5+,r1         ! 16 +
    mov.b   @r5+,r2         ! 17 +
    mov.b   @r5+,r3         ! 18 +
    add     #8 ,r4          ! 19 +
    mov.b   r3  ,@-r4       ! 20 +
    dt      r6              ! 21 +
    mov.b   r2  ,@-r4       ! 22 +
    mov.b   r1  ,@-r4       ! 23 +
    bf/s    3b              ! 24 +
    mov.b   r0  ,@-r4       ! 25 +
4:  rts                     ! 26
    mov     r7,r0           ! 27 return dst

#ifndef PRECEDING
.align 5
ENTRY(memcpy)
ENTRY(memcpyf)                     ! r4 = dst , r5 = src, r6 = size
    mov     r4,r7           ! 00
    mov     #0x7F,r3        ! 01
    mov     r5,r0           ! 02 r0 = src
    cmp/hi  r3,r6           ! 03 if less than 63 bytes
    or      r4,r0           ! 04 r0 = src | dst
    bf/s    .Llastword128   ! 05 less  than r3 unaligned small byte copy
    tst     #3,r0           ! 06 if ((src|dst)&3)==0
    mov.l   r4,@-r15        ! 07 save dst into stack
    neg     r4,r0           ! 08
    shlr    r0              ! 09
    bf/s    1f              ! 10
    shlr    r0              ! 11
    mov.b   @r5+,r1         ! 12
    add     #-1 ,r6         ! 13
    mov.b   r1  ,@r4        ! 14
    add     #1  ,r4         ! 15
1:  bf      2f              ! 16
    mov.b   @r5+,r1         ! 17
    mov.b   @r5+,r0         ! 18
    add     #-2 ,r6         ! 19
    mov.b   r1  ,@r4        ! 20
    mov.b   r0  ,@(1,r4)    ! 21
    add     #2,r4           ! 22
2:  mov     #0x1c,r3        ! 23
    mov      r5,r0          ! 24
    neg      r4,r1          ! 25
    tst      #3,r0          ! 26
    and      r1,r3          ! 27 r3 = num words to move
    mov      r6,r7          ! 28
    shlr2    r3             ! 29 r3 = num words to move to be 32 bytes aligned 0,1,2,3,4,5,6,7
    shlr2    r7             ! 30
    bt/s    .Lbigmove32     ! 31 dst&3=0 -> aligned : word copy 4 by 4
    tst      r3,r3          ! 32
    tst      #1,r0          ! 33
    bt/s    .Lbigmove16     ! 34 dst&1=0  -> short aligned : word copy 2-2
    tst     r3  , r3        ! 35
    bra     .Lbigmove8      ! 36  dst&1=1 -> byte shift
    mov.l   r8 ,@-r15       ! 37
#endif

#ifdef MEMCPY_USES_FPU
.align 5
.Lbigmove32:                /* r3 = num to be aligned r4 = dst , r5 = src, r6 = size, r7= numwords  */
    bt/s    3f              ! 00
    mov     #-3 ,r0         ! 01
    sub     r3  ,r7         ! 02
2:  mov.l   @r5+,r1         ! 03
    dt      r3              ! 04
    mov.l   r1  ,@r4        ! 05
    bf/s    2b              ! 06
    add     #4  ,r4         ! 07  src and dst are aligned on 8 bytes
3:  mov     r7  ,r2         ! 08  move by chunk of 32 bytes = r7 is number of words
    shld    r0  ,r2         ! 09
    add     #-4 ,r2         ! 10 r2 = word/8 - 4 ,number of chunks we can move with prefetchs
    cmp/pl  r2              ! 11 if !(r2>0)
    bf      5f              ! 12 go to loop move if less than 128 bytes to move
    mov     #7,r0           ! 13
    and     r0 ,r7          ! 14  we need to compute the resulting number of words to move without prefetching
    add     #32,r7          ! 15
    mov     r5  ,r0         ! 16
    mov     r5  ,r1         ! 17
    tst     #4  ,r0         ! 18
    bt/s    31f             ! 19
    add     #127,r1         ! 20
    add     #-28,r4         ! 21
4:  mov.l   @r5+,r0         ! 22 +
    fmov.s  @r5+,fr1        ! 23 +
    fmov.s  @r5+,fr2        ! 24 +
    fmov.s  @r5+,fr3        ! 25 +
    fmov.s  @r5+,fr4        ! 26 +
    fmov.s  @r5+,fr5        ! 27 +
    fmov.s  @r5+,fr6        ! 28 +
    fmov.s  @r5+,fr7        ! 29 +
    PREF   (@r1)            ! 30 +
    add     #28 ,r4         ! 31 +
    CACHE   r0  ,@r4        ! 32 +
    add     #32 ,r4         ! 33 +
    fmov.s  fr7 ,@-r4       ! 34 +
    add     #32 ,r1         ! 35 +
    fmov.s  fr6 ,@-r4       ! 36 +
    dt      r2              ! 37 +
    fmov.s  fr5 ,@-r4       ! 38 +
    fmov.s  fr4 ,@-r4       ! 39 +
    fmov.s  fr3 ,@-r4       ! 40 +
    fmov.s  fr2 ,@-r4       ! 41 +
    bf/s    4b              ! 42 +
    fmov.s  fr1 ,@-r4       ! 43 +
    bra 5f                  ! 44
    add     #28 ,r4         ! 45
31: mov.l   .L1 ,r0         ! 46
    sts     fpscr   ,r3     ! 47  r3 : shadow of FPCU
    add     #-32,r4         ! 48
    lds     r0  ,fpscr      ! 49
41: fmov    @r5+,dr0        ! 50 +
    fmov    @r5+,dr2        ! 51 +
    fmov    @r5+,dr4        ! 52 +
    fmov    @r5+,dr6        ! 53 +
    PREF   (@r1)            ! 54 +
    add     #32 ,r4         ! 55 +
    CACHE   r0  ,@r4        ! 56 +
    add     #32 ,r4         ! 57 +
    fmov    dr6 ,@-r4       ! 58 +
    add     #32 ,r1         ! 59 +
    fmov    dr4 ,@-r4       ! 60 +
    dt      r2              ! 61 +
    fmov    dr2 ,@-r4       ! 62 +
    bf/s    41b             ! 63 +
    fmov    dr0 ,@-r4       ! 64 +
    lds     r3, fpscr       ! 65
    add     #32 ,r4         ! 66
5:  tst     r7, r7          ! 67 r7 remaining words, number from 0 to 31
    bt      7f              ! 68
6:  mov.l   @r5+,r1         ! 69
    dt      r7              ! 70
    mov.l   r1  ,@r4        ! 71
    bf/s    6b              ! 72
    add     #4  ,r4         ! 73
7:  mov     r6  ,r0         ! 74  r0 = remaining last bytes
    tst     #3  ,r0         ! 75
    bt/s    9f              ! 76
    and     #3  ,r0         ! 77
8:  mov.b   @r5+,r1         ! 78
    dt      r0              ! 79
    mov.b   r1  ,@r4        ! 80
    bf/s    8b              ! 81
    add     #1  ,r4         ! 82
9:  rts                     ! 83
    mov.l   @r15+,r0        ! 84
#else
.align 5
.Lbigmove32:                /* r3 = num to be aligned r4 = dst , r5 = src, r6 = size, r7= numwords  */
    bt/s    3f              ! 00
    mov     #-3 ,r0         ! 01
    sub     r3  ,r7         ! 02
2:  mov.l   @r5+,r1         ! 03
    dt      r3              ! 04
    mov.l   r1  ,@r4        ! 05
    bf/s    2b              ! 06
    add     #4  ,r4         ! 07 src and dst are aligned on 8 bytes
3:  mov     r7  ,r2         ! 08 move by chunk of 32 bytes = r7 is number of words
    shld    r0  ,r2         ! 09 r2 = words/8
    add     #-4 ,r2         ! 10 r2 = word/8 - 4 ,number of chunks we can move with prefetchs
    cmp/pl  r2              ! 11 if !(r2>0)
    bf      5f              ! 12 go to loop move if less than 128 bytes to move
    mov     r5  ,r1         ! 13
    mov.l   r8 ,@-r15       ! 14
    add     #127,r1         ! 15
    mov.l   r9 ,@-r15       ! 16
    mov     #7,r0           ! 17
    mov.l   r10,@-r15       ! 18
    and     r0 ,r7          ! 19  we need to compute the resulting number of words to move without prefetching
    mov.l   r11,@-r15       ! 20
    add     #32,r7          ! 21
    mov.l   r12,@-r15       ! 22
    mov.l   r13,@-r15       ! 23
    mov.l   r14,@-r15       ! 24
    add     #-32,r4         ! 25
4:  mov.l   @r5+ ,r0        ! 26  +
    mov.l   @r5+ ,r8        ! 27  +
    mov.l   @r5+ ,r9        ! 28  +
    mov.l   @r5+ ,r10       ! 29  +
    mov.l   @r5+ ,r11       ! 30  +
    mov.l   @r5+ ,r12       ! 31  +
    mov.l   @r5+ ,r13       ! 32  +
    mov.l   @r5+ ,r14       ! 33  +
    PREF   (@r1)            ! 35  +
    add     #32  ,r4        ! 34  +
    CACHE   r0   ,@r4       ! 36  +
    mov.l   r8   ,@(4,r4)   ! 37  +
    add     #32  ,r1        ! 38  +
    mov.l   r9   ,@(8,r4)   ! 39  +
    dt      r2              ! 40  +
    mov.l   r10  ,@(12,r4)  ! 41  +
    mov.l   r11  ,@(16,r4)  ! 42  +
    mov.l   r12  ,@(20,r4)  ! 43  +
    mov.l   r13  ,@(24,r4)  ! 44  +
    bf/s    4b              ! 45  +
    mov.l   r14  ,@(28,r4)  ! 46  +
    add     #32  ,r4        ! 47
    mov.l   @r15+,r14       ! 48
    mov.l   @r15+,r13       ! 49
    mov.l   @r15+,r12       ! 50
    mov.l   @r15+,r11       ! 51
    mov.l   @r15+,r10       ! 52
    mov.l   @r15+,r9        ! 53
    mov.l   @r15+,r8        ! 54
5:  tst     r7, r7          ! 55 r7 remaining words, number from 0 to 31
    bt      7f              ! 56
6:  mov.l   @r5+ ,r1        ! 57
    dt      r7              ! 58
    mov.l   r1   ,@r4       ! 59
    bf/s    6b              ! 60
    add     #4   ,r4        ! 61
7:  mov     r6   ,r0        ! 62 r0 = remaining last bytes
    tst     #3   ,r0        ! 63
    bt/s    9f              ! 64
    and     #3  ,r0         ! 65
8:  mov.b   @r5+,r1         ! 66
    dt      r0              ! 67
    mov.b   r1  ,@r4        ! 68
    bf/s    8b              ! 69
    add     #1  ,r4         ! 70
9:  rts                     ! 71
    mov.l   @r15+,r0        ! 72
#endif

.align 5
.Lbigmove16:                ! r3 : num words to be aligned r4 = dst , r5 = src, r6 = size, r7= numwords
    mov.w   @r5+,r0         ! 00    read first 2 bytes->r0
    mov     #-3,r1          ! 01
    bt/s    3f              ! 02
    shll16  r0              ! 03 : xx--
    sub     r3  ,r7         ! 04
2:  mov.l   @r5+,r2         ! 05
    dt      r3              ! 06
    xtrct   r2  ,r0         ! 07
    mov.l   r0  ,@r4        ! 08
    add     #4  ,r4         ! 09  src and dst are aligned on 8 bytes
    bf/s    2b              ! 10
    mov     r2  ,r0         ! 11
3:  mov     r7  ,r2         ! 12  move by chunk of 32 bytes = r7 is number of words
    shld    r1  ,r2         ! 13
    add     #-4 ,r2         ! 14 r2 = word/8 - 4 ,number of chunks we can move with prefetchs
    cmp/pl  r2              ! 15 if !(r2>0)
    bf      5f              ! 16 go to loop move if less than 128 bytes to move
    mov     #7 ,r1          ! 17
    and     r1 ,r7          ! 18  we need to compute the resulting number of words to move without prefetching
    add     #32,r7          ! 19
    mov     r5  ,r3         ! 20
    mov.l   r8 ,@-r15       ! 21
    add     #127,r3         ! 22
    mov.l   r9 ,@-r15       ! 23
    add     #-32,r4         ! 24
    mov.l   r10,@-r15       ! 25
    mov.l   r11,@-r15       ! 26
    mov.l   r12,@-r15       ! 27
    mov.l   r13,@-r15       ! 28
4:  mov.l   @r5+,r1         ! 29 +
    mov.l   @r5+,r8         ! 30 +
    mov.l   @r5+,r9         ! 31 +
    mov.l   @r5+,r10        ! 32 +
    mov.l   @r5+,r11        ! 33 +
    dt      r2              ! 34 +
    mov.l   @r5+,r12        ! 35 +
    add     #32,r4          ! 36 +
    mov.l   @r5+,r13        ! 37 +
    xtrct   r1 ,r0          ! 38 +
    PREF   (@r3)            ! 39 +
    xtrct   r8 ,r1          ! 40 +
    CACHE   r0,@r4          ! 41 +
    xtrct   r9 ,r8          ! 42 +
    mov.l   @r5+,r0         ! 43 +
    add     #32 ,r3         ! 44 +
    mov.l   r1  ,@(4,r4)    ! 45 +
    xtrct   r10,r9          ! 46 +
    mov.l   r8  ,@(8,r4)    ! 47 +
    xtrct   r11,r10         ! 48 +
    mov.l   r9  ,@(12,r4)   ! 49 +
    xtrct   r12,r11         ! 50 +
    mov.l   r10 ,@(16,r4)   ! 51 +
    xtrct   r13,r12         ! 52 +
    mov.l   r11 ,@(20,r4)   ! 53 +
    xtrct   r0 ,r13         ! 54 +
    mov.l   r12 ,@(24,r4)   ! 55 +
    bf/s    4b              ! 56 +
    mov.l   r13 ,@(28,r4)   ! 57 +
    add     #32,r4          ! 58
    mov.l   @r15+,r13       ! 59
    mov.l   @r15+,r12       ! 60
    mov.l   @r15+,r11       ! 61
    mov.l   @r15+,r10       ! 62
    mov.l   @r15+,r9        ! 63
    mov.l   @r15+,r8        ! 64
5:  tst     r7, r7          ! 65 r7 remaining words, number from 0 to 31
    bt      7f              ! 66
6:  mov.l   @r5+,r2         ! 67
    dt      r7              ! 68
    xtrct   r2  ,r0         ! 69
    mov.l   r0  ,@r4        ! 70
    add     #4  ,r4         ! 71 src and dst are aligned on 8 bytes
    bf/s    6b              ! 72
    mov     r2  ,r0         ! 73
7:  mov     r6  ,r0         ! 74 r0 = remaining last bytes
    tst     #3  ,r0         ! 75
    add     #-2 ,r5         ! 76
    bt      9f              ! 77
    and     #3  ,r0         ! 78
8:  mov.b   @r5+,r1         ! 79
    dt      r0              ! 80
    mov.b   r1  ,@r4        ! 81
    bf/s    8b              ! 82
    add     #1  ,r4         ! 83
9:  rts                     ! 84
    mov.l   @r15+,r0        ! 85

.align 2
.L1:
    .long 0x00100000




