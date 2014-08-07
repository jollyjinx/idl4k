/*****************************************************************************

File name   :  osdclut.h

Description :  OSD macro, and pre-defined parameters

 COPYRIGHT (C) FTA Communication techniologies 2007.

Date               Modification                 Name
----               ------------                 ----
 12/09/00          Created                      FR

*****************************************************************************/
/* --- prevents recursive inclusion --------------------------------------- */
#ifndef __OSDMAC_H
#define __OSDMAC_H

/* --- allows C compiling with C++ compiler ------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ----------------------------------------------------------- */

/* --- defines ------------------------------------------------------------ */

/* --- variables ---------------------------------------------------------- */
/* =======================================================================
   This is the predefined CLUT for OSD look-up table
   ======================================================================== */
U32 OSDClut888         [256]=
   {
    0x00000000, /*                 transparency    =0       */
    0x00ffffff, /*                 white           =1       */
    0x00000000, /*                 black           =2       */
    0x004080c0, /*                 blue ST         =3       */
    0x00808080, /*                 Font1 Alias1    =4       */
    0x00808080, /*                 Font2 Alias1    =5       */
    0x00808080, /*                 Font3 Alias1    =6       */
    0x00808080, /*                 Font1 Alias2    =7       */
    0x00808080, /*                 Font2 Alias2    =8       */
    0x00808080, /*                 Font3 Alias2    =9       */
    0x00000000, /*                 Font1 Black     =10      */
    0x00000000, /*                 Font2 Black     =11      */
    0x00000000, /*                 Font3 Black     =12      */
    0x00ffffff, /*                 Font1 White     =13      */
    0x00ffffff, /*                 Font2 White     =14      */
    0x00ffffff, /*                 Font3 White     =15      */
    0x00000000, /*                 Full black      =16      */
    0x000000ff, /*                 Full blue       =17      */
    0x00ff0000, /*                 Full red        =18      */
    0x00ff00ff, /*                 Full magenta    =19      */
    0x0000ff00, /*                 Full green      =20      */
    0x0000ffff, /*                 Full cyan       =21      */
    0x00ffff00, /*                 Full yellow     =22      */
    0x00ffffff, /*                 Full white      =23      */
    0x00080808, /*            Grey level   8, ref = 16      */
    0x00181818, /*            Grey level  24, ref = 17      */
    0x00282828, /*            Grey level  40, ref = 18      */
    0x00383838, /*            Grey level  56, ref = 19      */
    0x00484848, /*            Grey level  72, ref = 20      */
    0x00585858, /*            Grey level  88, ref = 21      */
    0x00686868, /*            Grey level 104, ref = 22      */
    0x00787878, /*            Grey level 120, ref = 23      */
    0x00888888, /*            Grey level 136, ref = 24      */
    0x00989898, /*            Grey level 152, ref = 25      */
    0x00a8a8a8, /*            Grey level 168, ref = 26      */
    0x00b8b8b8, /*            Grey level 184, ref = 27      */
    0x00c8c8c8, /*            Grey level 200, ref = 28      */
    0x00d8d8d8, /*            Grey level 216, ref = 29      */
    0x00e8e8e8, /*            Grey level 232, ref = 30      */
    0x00f8f8f8, /*            Grey level 248, ref = 31      */
    0x00161616, /*         R= 22, G= 22, B= 22, mix=63      */
    0x00161640, /*         R= 22, G= 22, B= 64, mix=63      */
    0x0016166a, /*         R= 22, G= 22, B=106, mix=63      */
    0x00161694, /*         R= 22, G= 22, B=148, mix=63      */
    0x001616be, /*         R= 22, G= 22, B=190, mix=63      */
    0x001616e8, /*         R= 22, G= 22, B=232, mix=63      */
    0x00164016, /*         R= 22, G= 64, B= 22, mix=63      */
    0x00164040, /*         R= 22, G= 64, B= 64, mix=63      */
    0x0016406a, /*         R= 22, G= 64, B=106, mix=63      */
    0x00164094, /*         R= 22, G= 64, B=148, mix=63      */
    0x001640be, /*         R= 22, G= 64, B=190, mix=63      */
    0x001640e8, /*         R= 22, G= 64, B=232, mix=63      */
    0x00166a16, /*         R= 22, G=106, B= 22, mix=63      */
    0x00166a40, /*         R= 22, G=106, B= 64, mix=63      */
    0x00166a6a, /*         R= 22, G=106, B=106, mix=63      */
    0x00166a94, /*         R= 22, G=106, B=148, mix=63      */
    0x00166abe, /*         R= 22, G=106, B=190, mix=63      */
    0x00166ae8, /*         R= 22, G=106, B=232, mix=63      */
    0x00169416, /*         R= 22, G=148, B= 22, mix=63      */
    0x00169440, /*         R= 22, G=148, B= 64, mix=63      */
    0x0016946a, /*         R= 22, G=148, B=106, mix=63      */
    0x00169494, /*         R= 22, G=148, B=148, mix=63      */
    0x001694be, /*         R= 22, G=148, B=190, mix=63      */
    0x001694e8, /*         R= 22, G=148, B=232, mix=63      */
    0x0016be16, /*         R= 22, G=190, B= 22, mix=63      */
    0x0016be40, /*         R= 22, G=190, B= 64, mix=63      */
    0x0016be6a, /*         R= 22, G=190, B=106, mix=63      */
    0x0016be94, /*         R= 22, G=190, B=148, mix=63      */
    0x0016bebe, /*         R= 22, G=190, B=190, mix=63      */
    0x0016bee8, /*         R= 22, G=190, B=232, mix=63      */
    0x0016e816, /*         R= 22, G=232, B= 22, mix=63      */
    0x0016e840, /*         R= 22, G=232, B= 64, mix=63      */
    0x0016e86a, /*         R= 22, G=232, B=106, mix=63      */
    0x0016e894, /*         R= 22, G=232, B=148, mix=63      */
    0x0016e8be, /*         R= 22, G=232, B=190, mix=63      */
    0x0016e8e8, /*         R= 22, G=232, B=232, mix=63      */
    0x00401616, /*         R= 64, G= 22, B= 22, mix=63      */
    0x00401640, /*         R= 64, G= 22, B= 64, mix=63      */
    0x0040166a, /*         R= 64, G= 22, B=106, mix=63      */
    0x00401694, /*         R= 64, G= 22, B=148, mix=63      */
    0x004016be, /*         R= 64, G= 22, B=190, mix=63      */
    0x004016e8, /*         R= 64, G= 22, B=232, mix=63      */
    0x00404016, /*         R= 64, G= 64, B= 22, mix=63      */
    0x00404040, /*         R= 64, G= 64, B= 64, mix=63      */
    0x0040406a, /*         R= 64, G= 64, B=106, mix=63      */
    0x00404094, /*         R= 64, G= 64, B=148, mix=63      */
    0x004040be, /*         R= 64, G= 64, B=190, mix=63      */
    0x004040e8, /*         R= 64, G= 64, B=232, mix=63      */
    0x00406a16, /*         R= 64, G=106, B= 22, mix=63      */
    0x00406a40, /*         R= 64, G=106, B= 64, mix=63      */
    0x00406a6a, /*         R= 64, G=106, B=106, mix=63      */
    0x00406a94, /*         R= 64, G=106, B=148, mix=63      */
    0x00406abe, /*         R= 64, G=106, B=190, mix=63      */
    0x00406ae8, /*         R= 64, G=106, B=232, mix=63      */
    0x00409416, /*         R= 64, G=148, B= 22, mix=63      */
    0x00409440, /*         R= 64, G=148, B= 64, mix=63      */
    0x0040946a, /*         R= 64, G=148, B=106, mix=63      */
    0x00409494, /*         R= 64, G=148, B=148, mix=63      */
    0x004094be, /*         R= 64, G=148, B=190, mix=63      */
    0x004094e8, /*         R= 64, G=148, B=232, mix=63      */
    0x0040be16, /*         R= 64, G=190, B= 22, mix=63      */
    0x0040be40, /*         R= 64, G=190, B= 64, mix=63      */
    0x0040be6a, /*         R= 64, G=190, B=106, mix=63      */
    0x0040be94, /*         R= 64, G=190, B=148, mix=63      */
    0x0040bebe, /*         R= 64, G=190, B=190, mix=63      */
    0x0040bee8, /*         R= 64, G=190, B=232, mix=63      */
    0x0040e816, /*         R= 64, G=232, B= 22, mix=63      */
    0x0040e840, /*         R= 64, G=232, B= 64, mix=63      */
    0x0040e86a, /*         R= 64, G=232, B=106, mix=63      */
    0x0040e894, /*         R= 64, G=232, B=148, mix=63      */
    0x0040e8be, /*         R= 64, G=232, B=190, mix=63      */
    0x0040e8e8, /*         R= 64, G=232, B=232, mix=63      */
    0x006a1616, /*         R=106, G= 22, B= 22, mix=63      */
    0x006a1640, /*         R=106, G= 22, B= 64, mix=63      */
    0x006a166a, /*         R=106, G= 22, B=106, mix=63      */
    0x006a1694, /*         R=106, G= 22, B=148, mix=63      */
    0x006a16be, /*         R=106, G= 22, B=190, mix=63      */
    0x006a16e8, /*         R=106, G= 22, B=232, mix=63      */
    0x006a4016, /*         R=106, G= 64, B= 22, mix=63      */
    0x006a4040, /*         R=106, G= 64, B= 64, mix=63      */
    0x006a406a, /*         R=106, G= 64, B=106, mix=63      */
    0x006a4094, /*         R=106, G= 64, B=148, mix=63      */
    0x006a40be, /*         R=106, G= 64, B=190, mix=63      */
    0x006a40e8, /*         R=106, G= 64, B=232, mix=63      */
    0x006a6a16, /*         R=106, G=106, B= 22, mix=63      */
    0x006a6a40, /*         R=106, G=106, B= 64, mix=63      */
    0x006a6a6a, /*         R=106, G=106, B=106, mix=63      */
    0x006a6a94, /*         R=106, G=106, B=148, mix=63      */
    0x006a6abe, /*         R=106, G=106, B=190, mix=63      */
    0x006a6ae8, /*         R=106, G=106, B=232, mix=63      */
    0x006a9416, /*         R=106, G=148, B= 22, mix=63      */
    0x006a9440, /*         R=106, G=148, B= 64, mix=63      */
    0x006a946a, /*         R=106, G=148, B=106, mix=63      */
    0x006a9494, /*         R=106, G=148, B=148, mix=63      */
    0x006a94be, /*         R=106, G=148, B=190, mix=63      */
    0x006a94e8, /*         R=106, G=148, B=232, mix=63      */
    0x006abe16, /*         R=106, G=190, B= 22, mix=63      */
    0x006abe40, /*         R=106, G=190, B= 64, mix=63      */
    0x006abe6a, /*         R=106, G=190, B=106, mix=63      */
    0x006abe94, /*         R=106, G=190, B=148, mix=63      */
    0x006abebe, /*         R=106, G=190, B=190, mix=63      */
    0x006abee8, /*         R=106, G=190, B=232, mix=63      */
    0x006ae816, /*         R=106, G=232, B= 22, mix=63      */
    0x006ae840, /*         R=106, G=232, B= 64, mix=63      */
    0x006ae86a, /*         R=106, G=232, B=106, mix=63      */
    0x006ae894, /*         R=106, G=232, B=148, mix=63      */
    0x006ae8be, /*         R=106, G=232, B=190, mix=63      */
    0x006ae8e8, /*         R=106, G=232, B=232, mix=63      */
    0x00941616, /*         R=148, G= 22, B= 22, mix=63      */
    0x00941640, /*         R=148, G= 22, B= 64, mix=63      */
    0x0094166a, /*         R=148, G= 22, B=106, mix=63      */
    0x00941694, /*         R=148, G= 22, B=148, mix=63      */
    0x009416be, /*         R=148, G= 22, B=190, mix=63      */
    0x009416e8, /*         R=148, G= 22, B=232, mix=63      */
    0x00944016, /*         R=148, G= 64, B= 22, mix=63      */
    0x00944040, /*         R=148, G= 64, B= 64, mix=63      */
    0x0094406a, /*         R=148, G= 64, B=106, mix=63      */
    0x00944094, /*         R=148, G= 64, B=148, mix=63      */
    0x009440be, /*         R=148, G= 64, B=190, mix=63      */
    0x009440e8, /*         R=148, G= 64, B=232, mix=63      */
    0x00946a16, /*         R=148, G=106, B= 22, mix=63      */
    0x00946a40, /*         R=148, G=106, B= 64, mix=63      */
    0x00946a6a, /*         R=148, G=106, B=106, mix=63      */
    0x00946a94, /*         R=148, G=106, B=148, mix=63      */
    0x00946abe, /*         R=148, G=106, B=190, mix=63      */
    0x00946ae8, /*         R=148, G=106, B=232, mix=63      */
    0x00949416, /*         R=148, G=148, B= 22, mix=63      */
    0x00949440, /*         R=148, G=148, B= 64, mix=63      */
    0x0094946a, /*         R=148, G=148, B=106, mix=63      */
    0x00949494, /*         R=148, G=148, B=148, mix=63      */
    0x009494be, /*         R=148, G=148, B=190, mix=63      */
    0x009494e8, /*         R=148, G=148, B=232, mix=63      */
    0x0094be16, /*         R=148, G=190, B= 22, mix=63      */
    0x0094be40, /*         R=148, G=190, B= 64, mix=63      */
    0x0094be6a, /*         R=148, G=190, B=106, mix=63      */
    0x0094be94, /*         R=148, G=190, B=148, mix=63      */
    0x0094bebe, /*         R=148, G=190, B=190, mix=63      */
    0x0094bee8, /*         R=148, G=190, B=232, mix=63      */
    0x0094e816, /*         R=148, G=232, B= 22, mix=63      */
    0x0094e840, /*         R=148, G=232, B= 64, mix=63      */
    0x0094e86a, /*         R=148, G=232, B=106, mix=63      */
    0x0094e894, /*         R=148, G=232, B=148, mix=63      */
    0x0094e8be, /*         R=148, G=232, B=190, mix=63      */
    0x0094e8e8, /*         R=148, G=232, B=232, mix=63      */
    0x00be1616, /*         R=190, G= 22, B= 22, mix=63      */
    0x00be1640, /*         R=190, G= 22, B= 64, mix=63      */
    0x00be166a, /*         R=190, G= 22, B=106, mix=63      */
    0x00be1694, /*         R=190, G= 22, B=148, mix=63      */
    0x00be16be, /*         R=190, G= 22, B=190, mix=63      */
    0x00be16e8, /*         R=190, G= 22, B=232, mix=63      */
    0x00be4016, /*         R=190, G= 64, B= 22, mix=63      */
    0x00be4040, /*         R=190, G= 64, B= 64, mix=63      */
    0x00be406a, /*         R=190, G= 64, B=106, mix=63      */
    0x00be4094, /*         R=190, G= 64, B=148, mix=63      */
    0x00be40be, /*         R=190, G= 64, B=190, mix=63      */
    0x00be40e8, /*         R=190, G= 64, B=232, mix=63      */
    0x00be6a16, /*         R=190, G=106, B= 22, mix=63      */
    0x00be6a40, /*         R=190, G=106, B= 64, mix=63      */
    0x00be6a6a, /*         R=190, G=106, B=106, mix=63      */
    0x00be6a94, /*         R=190, G=106, B=148, mix=63      */
    0x00be6abe, /*         R=190, G=106, B=190, mix=63      */
    0x00be6ae8, /*         R=190, G=106, B=232, mix=63      */
    0x00be9416, /*         R=190, G=148, B= 22, mix=63      */
    0x00be9440, /*         R=190, G=148, B= 64, mix=63      */
    0x00be946a, /*         R=190, G=148, B=106, mix=63      */
    0x00be9494, /*         R=190, G=148, B=148, mix=63      */
    0x00be94be, /*         R=190, G=148, B=190, mix=63      */
    0x00be94e8, /*         R=190, G=148, B=232, mix=63      */
    0x00bebe16, /*         R=190, G=190, B= 22, mix=63      */
    0x00bebe40, /*         R=190, G=190, B= 64, mix=63      */
    0x00bebe6a, /*         R=190, G=190, B=106, mix=63      */
    0x00bebe94, /*         R=190, G=190, B=148, mix=63      */
    0x00bebebe, /*         R=190, G=190, B=190, mix=63      */
    0x00bebee8, /*         R=190, G=190, B=232, mix=63      */
    0x00bee816, /*         R=190, G=232, B= 22, mix=63      */
    0x00bee840, /*         R=190, G=232, B= 64, mix=63      */
    0x00bee86a, /*         R=190, G=232, B=106, mix=63      */
    0x00bee894, /*         R=190, G=232, B=148, mix=63      */
    0x00bee8be, /*         R=190, G=232, B=190, mix=63      */
    0x00bee8e8, /*         R=190, G=232, B=232, mix=63      */
    0x00e81616, /*         R=232, G= 22, B= 22, mix=63      */
    0x00e81640, /*         R=232, G= 22, B= 64, mix=63      */
    0x00e8166a, /*         R=232, G= 22, B=106, mix=63      */
    0x00e81694, /*         R=232, G= 22, B=148, mix=63      */
    0x00e816be, /*         R=232, G= 22, B=190, mix=63      */
    0x00e816e8, /*         R=232, G= 22, B=232, mix=63      */
    0x00e84016, /*         R=232, G= 64, B= 22, mix=63      */
    0x00e84040, /*         R=232, G= 64, B= 64, mix=63      */
    0x00e8406a, /*         R=232, G= 64, B=106, mix=63      */
    0x00e84094, /*         R=232, G= 64, B=148, mix=63      */
    0x00e840be, /*         R=232, G= 64, B=190, mix=63      */
    0x00e840e8, /*         R=232, G= 64, B=232, mix=63      */
    0x00e86a16, /*         R=232, G=106, B= 22, mix=63      */
    0x00e86a40, /*         R=232, G=106, B= 64, mix=63      */
    0x00e86a6a, /*         R=232, G=106, B=106, mix=63      */
    0x00e86a94, /*         R=232, G=106, B=148, mix=63      */
    0x00e86abe, /*         R=232, G=106, B=190, mix=63      */
    0x00e86ae8, /*         R=232, G=106, B=232, mix=63      */
    0x00e89416, /*         R=232, G=148, B= 22, mix=63      */
    0x00e89440, /*         R=232, G=148, B= 64, mix=63      */
    0x00e8946a, /*         R=232, G=148, B=106, mix=63      */
    0x00e89494, /*         R=232, G=148, B=148, mix=63      */
    0x00e894be, /*         R=232, G=148, B=190, mix=63      */
    0x00e894e8, /*         R=232, G=148, B=232, mix=63      */
    0x00e8be16, /*         R=232, G=190, B= 22, mix=63      */
    0x00e8be40, /*         R=232, G=190, B= 64, mix=63      */
    0x00e8be6a, /*         R=232, G=190, B=106, mix=63      */
    0x00e8be94, /*         R=232, G=190, B=148, mix=63      */
    0x00e8bebe, /*         R=232, G=190, B=190, mix=63      */
    0x00e8bee8, /*         R=232, G=190, B=232, mix=63      */
    0x00e8e816, /*         R=232, G=232, B= 22, mix=63      */
    0x00e8e840, /*         R=232, G=232, B= 64, mix=63      */
    0x00e8e86a, /*         R=232, G=232, B=106, mix=63      */
    0x00e8e894, /*         R=232, G=232, B=148, mix=63      */
    0x00e8e8be, /*         R=232, G=232, B=190, mix=63      */
    0x00e8e8e8, /*         R=232, G=232, B=232, mix=63      */
};
/****************************************************************************/
U32 OSDClutCbYCrY[256]=
{
    0x10801080, /* ClutCbYCrY         transparency    =0        */
    0xeb80eb80, /* ClutCbYCrY         white           =1        */
    0x10801080, /* ClutCbYCrY         black           =2        */
    0x736073a5, /* ClutCbYCrY         blue ST         =3        */
    0x7d807d80, /* ClutCbYCrY         Font1 Alias1    =4        */
    0x7d807d80, /* ClutCbYCrY         Font2 Alias1    =5        */
    0x7d807d80, /* ClutCbYCrY         Font3 Alias1    =6        */
    0x7d807d80, /* ClutCbYCrY         Font1 Alias2    =7        */
    0x7d807d80, /* ClutCbYCrY         Font2 Alias2    =8        */
    0x7d807d80, /* ClutCbYCrY         Font3 Alias2    =9        */
    0x10801080, /* ClutCbYCrY         Font1 Black     =10       */
    0x10801080, /* ClutCbYCrY         Font2 Black     =11       */
    0x10801080, /* ClutCbYCrY         Font3 Black     =12       */
    0xeb80eb80, /* ClutCbYCrY         Font1 White     =13       */
    0xeb80eb80, /* ClutCbYCrY         Font2 White     =14       */
    0xeb80eb80, /* ClutCbYCrY         Font3 White     =15       */
    0x10801080, /* ClutCbYCrY         Full black      =16       */
    0x286e28ef, /* ClutCbYCrY         Full blue       =17       */
    0x51ef515b, /* ClutCbYCrY         Full red        =18       */
    0x6add6aca, /* ClutCbYCrY         Full magenta    =19       */
    0x90239036, /* ClutCbYCrY         Full green      =20       */
    0xa911a9a5, /* ClutCbYCrY         Full cyan       =21       */
    0xd292d211, /* ClutCbYCrY         Full yellow     =22       */
    0xeb80eb80, /* ClutCbYCrY         Full white      =23       */
    0x16801680, /* ClutCbYCrY    Grey level   8, ref = 16       */
    0x24802480, /* ClutCbYCrY    Grey level  24, ref = 17       */
    0x32803280, /* ClutCbYCrY    Grey level  40, ref = 18       */
    0x40804080, /* ClutCbYCrY    Grey level  56, ref = 19       */
    0x4d804d80, /* ClutCbYCrY    Grey level  72, ref = 20       */
    0x5b805b80, /* ClutCbYCrY    Grey level  88, ref = 21       */
    0x69806980, /* ClutCbYCrY    Grey level 104, ref = 22       */
    0x77807780, /* ClutCbYCrY    Grey level 120, ref = 23       */
    0x84808480, /* ClutCbYCrY    Grey level 136, ref = 24       */
    0x92809280, /* ClutCbYCrY    Grey level 152, ref = 25       */
    0xa080a080, /* ClutCbYCrY    Grey level 168, ref = 26       */
    0xae80ae80, /* ClutCbYCrY    Grey level 184, ref = 27       */
    0xbb80bb80, /* ClutCbYCrY    Grey level 200, ref = 28       */
    0xc980c980, /* ClutCbYCrY    Grey level 216, ref = 29       */
    0xd780d780, /* ClutCbYCrY    Grey level 232, ref = 30       */
    0xe580e580, /* ClutCbYCrY    Grey level 248, ref = 31       */
    0x22802280, /* ClutCbYCrY R= 22, G= 22, B= 22, mix=63       */
    0x277e2792, /* ClutCbYCrY R= 22, G= 22, B= 64, mix=63       */
    0x2b7b2ba4, /* ClutCbYCrY R= 22, G= 22, B=106, mix=63       */
    0x2f782fb7, /* ClutCbYCrY R= 22, G= 22, B=148, mix=63       */
    0x337533c9, /* ClutCbYCrY R= 22, G= 22, B=190, mix=63       */
    0x377237dc, /* ClutCbYCrY R= 22, G= 22, B=232, mix=63       */
    0x38713874, /* ClutCbYCrY R= 22, G= 64, B= 22, mix=63       */
    0x3c6e3c86, /* ClutCbYCrY R= 22, G= 64, B= 64, mix=63       */
    0x406b4098, /* ClutCbYCrY R= 22, G= 64, B=106, mix=63       */
    0x446844ab, /* ClutCbYCrY R= 22, G= 64, B=148, mix=63       */
    0x486548bd, /* ClutCbYCrY R= 22, G= 64, B=190, mix=63       */
    0x4c624ccf, /* ClutCbYCrY R= 22, G= 64, B=232, mix=63       */
    0x4d624d68, /* ClutCbYCrY R= 22, G=106, B= 22, mix=63       */
    0x515f517a, /* ClutCbYCrY R= 22, G=106, B= 64, mix=63       */
    0x555c558c, /* ClutCbYCrY R= 22, G=106, B=106, mix=63       */
    0x5959599e, /* ClutCbYCrY R= 22, G=106, B=148, mix=63       */
    0x5d565db1, /* ClutCbYCrY R= 22, G=106, B=190, mix=63       */
    0x615361c3, /* ClutCbYCrY R= 22, G=106, B=232, mix=63       */
    0x6252625c, /* ClutCbYCrY R= 22, G=148, B= 22, mix=63       */
    0x664f666e, /* ClutCbYCrY R= 22, G=148, B= 64, mix=63       */
    0x6a4c6a80, /* ClutCbYCrY R= 22, G=148, B=106, mix=63       */
    0x6e496e92, /* ClutCbYCrY R= 22, G=148, B=148, mix=63       */
    0x724672a5, /* ClutCbYCrY R= 22, G=148, B=190, mix=63       */
    0x764376b7, /* ClutCbYCrY R= 22, G=148, B=232, mix=63       */
    0x77437750, /* ClutCbYCrY R= 22, G=190, B= 22, mix=63       */
    0x7b407b62, /* ClutCbYCrY R= 22, G=190, B= 64, mix=63       */
    0x7f3d7f74, /* ClutCbYCrY R= 22, G=190, B=106, mix=63       */
    0x833a8386, /* ClutCbYCrY R= 22, G=190, B=148, mix=63       */
    0x88378898, /* ClutCbYCrY R= 22, G=190, B=190, mix=63       */
    0x8c348cab, /* ClutCbYCrY R= 22, G=190, B=232, mix=63       */
    0x8c338c43, /* ClutCbYCrY R= 22, G=232, B= 22, mix=63       */
    0x90309056, /* ClutCbYCrY R= 22, G=232, B= 64, mix=63       */
    0x942d9468, /* ClutCbYCrY R= 22, G=232, B=106, mix=63       */
    0x992a997b, /* ClutCbYCrY R= 22, G=232, B=148, mix=63       */
    0x9d279d8c, /* ClutCbYCrY R= 22, G=232, B=190, mix=63       */
    0xa124a19f, /* ClutCbYCrY R= 22, G=232, B=232, mix=63       */
    0x2d922d7a, /* ClutCbYCrY R= 64, G= 22, B= 22, mix=63       */
    0x318f318c, /* ClutCbYCrY R= 64, G= 22, B= 64, mix=63       */
    0x358c359e, /* ClutCbYCrY R= 64, G= 22, B=106, mix=63       */
    0x3a893ab1, /* ClutCbYCrY R= 64, G= 22, B=148, mix=63       */
    0x3e863ec3, /* ClutCbYCrY R= 64, G= 22, B=190, mix=63       */
    0x428342d5, /* ClutCbYCrY R= 64, G= 22, B=232, mix=63       */
    0x4282426e, /* ClutCbYCrY R= 64, G= 64, B= 22, mix=63       */
    0x46804680, /* ClutCbYCrY R= 64, G= 64, B= 64, mix=63       */
    0x4b7e4b92, /* ClutCbYCrY R= 64, G= 64, B=106, mix=63       */
    0x4f7b4fa4, /* ClutCbYCrY R= 64, G= 64, B=148, mix=63       */
    0x537853b7, /* ClutCbYCrY R= 64, G= 64, B=190, mix=63       */
    0x577557c9, /* ClutCbYCrY R= 64, G= 64, B=232, mix=63       */
    0x58745862, /* ClutCbYCrY R= 64, G=106, B= 22, mix=63       */
    0x5c715c74, /* ClutCbYCrY R= 64, G=106, B= 64, mix=63       */
    0x606e6086, /* ClutCbYCrY R= 64, G=106, B=106, mix=63       */
    0x646b6498, /* ClutCbYCrY R= 64, G=106, B=148, mix=63       */
    0x686868ab, /* ClutCbYCrY R= 64, G=106, B=190, mix=63       */
    0x6c656cbd, /* ClutCbYCrY R= 64, G=106, B=232, mix=63       */
    0x6d656d56, /* ClutCbYCrY R= 64, G=148, B= 22, mix=63       */
    0x71627168, /* ClutCbYCrY R= 64, G=148, B= 64, mix=63       */
    0x755f757a, /* ClutCbYCrY R= 64, G=148, B=106, mix=63       */
    0x795c798c, /* ClutCbYCrY R= 64, G=148, B=148, mix=63       */
    0x7d597d9e, /* ClutCbYCrY R= 64, G=148, B=190, mix=63       */
    0x815681b1, /* ClutCbYCrY R= 64, G=148, B=232, mix=63       */
    0x82558249, /* ClutCbYCrY R= 64, G=190, B= 22, mix=63       */
    0x8652865c, /* ClutCbYCrY R= 64, G=190, B= 64, mix=63       */
    0x8a4f8a6e, /* ClutCbYCrY R= 64, G=190, B=106, mix=63       */
    0x8e4c8e80, /* ClutCbYCrY R= 64, G=190, B=148, mix=63       */
    0x92499292, /* ClutCbYCrY R= 64, G=190, B=190, mix=63       */
    0x964696a5, /* ClutCbYCrY R= 64, G=190, B=232, mix=63       */
    0x9746973d, /* ClutCbYCrY R= 64, G=232, B= 22, mix=63       */
    0x9b439b50, /* ClutCbYCrY R= 64, G=232, B= 64, mix=63       */
    0x9f409f62, /* ClutCbYCrY R= 64, G=232, B=106, mix=63       */
    0xa33da374, /* ClutCbYCrY R= 64, G=232, B=148, mix=63       */
    0xa73aa786, /* ClutCbYCrY R= 64, G=232, B=190, mix=63       */
    0xac37ac98, /* ClutCbYCrY R= 64, G=232, B=232, mix=63       */
    0x38a43874, /* ClutCbYCrY R=106, G= 22, B= 22, mix=63       */
    0x3ca13c86, /* ClutCbYCrY R=106, G= 22, B= 64, mix=63       */
    0x409e4098, /* ClutCbYCrY R=106, G= 22, B=106, mix=63       */
    0x449b44aa, /* ClutCbYCrY R=106, G= 22, B=148, mix=63       */
    0x489848bd, /* ClutCbYCrY R=106, G= 22, B=190, mix=63       */
    0x4d954dcf, /* ClutCbYCrY R=106, G= 22, B=232, mix=63       */
    0x4d954d68, /* ClutCbYCrY R=106, G= 64, B= 22, mix=63       */
    0x5192517a, /* ClutCbYCrY R=106, G= 64, B= 64, mix=63       */
    0x558f558c, /* ClutCbYCrY R=106, G= 64, B=106, mix=63       */
    0x5a8c5a9e, /* ClutCbYCrY R=106, G= 64, B=148, mix=63       */
    0x5e895eb1, /* ClutCbYCrY R=106, G= 64, B=190, mix=63       */
    0x628662c3, /* ClutCbYCrY R=106, G= 64, B=232, mix=63       */
    0x6285625c, /* ClutCbYCrY R=106, G=106, B= 22, mix=63       */
    0x6682666e, /* ClutCbYCrY R=106, G=106, B= 64, mix=63       */
    0x6b806b80, /* ClutCbYCrY R=106, G=106, B=106, mix=63       */
    0x6f7e6f92, /* ClutCbYCrY R=106, G=106, B=148, mix=63       */
    0x737b73a4, /* ClutCbYCrY R=106, G=106, B=190, mix=63       */
    0x777877b7, /* ClutCbYCrY R=106, G=106, B=232, mix=63       */
    0x7777774f, /* ClutCbYCrY R=106, G=148, B= 22, mix=63       */
    0x7c747c62, /* ClutCbYCrY R=106, G=148, B= 64, mix=63       */
    0x80718074, /* ClutCbYCrY R=106, G=148, B=106, mix=63       */
    0x846e8486, /* ClutCbYCrY R=106, G=148, B=148, mix=63       */
    0x886b8898, /* ClutCbYCrY R=106, G=148, B=190, mix=63       */
    0x8c688cab, /* ClutCbYCrY R=106, G=148, B=232, mix=63       */
    0x8d688d43, /* ClutCbYCrY R=106, G=190, B= 22, mix=63       */
    0x91659156, /* ClutCbYCrY R=106, G=190, B= 64, mix=63       */
    0x95629568, /* ClutCbYCrY R=106, G=190, B=106, mix=63       */
    0x995f997a, /* ClutCbYCrY R=106, G=190, B=148, mix=63       */
    0x9d5c9d8c, /* ClutCbYCrY R=106, G=190, B=190, mix=63       */
    0xa159a19e, /* ClutCbYCrY R=106, G=190, B=232, mix=63       */
    0xa258a237, /* ClutCbYCrY R=106, G=232, B= 22, mix=63       */
    0xa655a649, /* ClutCbYCrY R=106, G=232, B= 64, mix=63       */
    0xaa52aa5c, /* ClutCbYCrY R=106, G=232, B=106, mix=63       */
    0xae4fae6e, /* ClutCbYCrY R=106, G=232, B=148, mix=63       */
    0xb24cb280, /* ClutCbYCrY R=106, G=232, B=190, mix=63       */
    0xb649b692, /* ClutCbYCrY R=106, G=232, B=232, mix=63       */
    0x43b7436e, /* ClutCbYCrY R=148, G= 22, B= 22, mix=63       */
    0x47b44780, /* ClutCbYCrY R=148, G= 22, B= 64, mix=63       */
    0x4bb14b92, /* ClutCbYCrY R=148, G= 22, B=106, mix=63       */
    0x4fae4fa4, /* ClutCbYCrY R=148, G= 22, B=148, mix=63       */
    0x53ab53b7, /* ClutCbYCrY R=148, G= 22, B=190, mix=63       */
    0x57a857c9, /* ClutCbYCrY R=148, G= 22, B=232, mix=63       */
    0x58a75862, /* ClutCbYCrY R=148, G= 64, B= 22, mix=63       */
    0x5ca45c74, /* ClutCbYCrY R=148, G= 64, B= 64, mix=63       */
    0x60a16086, /* ClutCbYCrY R=148, G= 64, B=106, mix=63       */
    0x649e6498, /* ClutCbYCrY R=148, G= 64, B=148, mix=63       */
    0x689b68aa, /* ClutCbYCrY R=148, G= 64, B=190, mix=63       */
    0x6d986dbd, /* ClutCbYCrY R=148, G= 64, B=232, mix=63       */
    0x6d986d55, /* ClutCbYCrY R=148, G=106, B= 22, mix=63       */
    0x71957168, /* ClutCbYCrY R=148, G=106, B= 64, mix=63       */
    0x7592757a, /* ClutCbYCrY R=148, G=106, B=106, mix=63       */
    0x798f798c, /* ClutCbYCrY R=148, G=106, B=148, mix=63       */
    0x7e8c7e9e, /* ClutCbYCrY R=148, G=106, B=190, mix=63       */
    0x828982b1, /* ClutCbYCrY R=148, G=106, B=232, mix=63       */
    0x82888249, /* ClutCbYCrY R=148, G=148, B= 22, mix=63       */
    0x8685865c, /* ClutCbYCrY R=148, G=148, B= 64, mix=63       */
    0x8b828b6e, /* ClutCbYCrY R=148, G=148, B=106, mix=63       */
    0x8f808f80, /* ClutCbYCrY R=148, G=148, B=148, mix=63       */
    0x937e9392, /* ClutCbYCrY R=148, G=148, B=190, mix=63       */
    0x977b97a4, /* ClutCbYCrY R=148, G=148, B=232, mix=63       */
    0x977a973d, /* ClutCbYCrY R=148, G=190, B= 22, mix=63       */
    0x9c779c4f, /* ClutCbYCrY R=148, G=190, B= 64, mix=63       */
    0xa074a062, /* ClutCbYCrY R=148, G=190, B=106, mix=63       */
    0xa471a474, /* ClutCbYCrY R=148, G=190, B=148, mix=63       */
    0xa86ea886, /* ClutCbYCrY R=148, G=190, B=190, mix=63       */
    0xac6bac98, /* ClutCbYCrY R=148, G=190, B=232, mix=63       */
    0xad6bad31, /* ClutCbYCrY R=148, G=232, B= 22, mix=63       */
    0xb168b143, /* ClutCbYCrY R=148, G=232, B= 64, mix=63       */
    0xb565b556, /* ClutCbYCrY R=148, G=232, B=106, mix=63       */
    0xb962b968, /* ClutCbYCrY R=148, G=232, B=148, mix=63       */
    0xbd5fbd7a, /* ClutCbYCrY R=148, G=232, B=190, mix=63       */
    0xc15cc18c, /* ClutCbYCrY R=148, G=232, B=232, mix=63       */
    0x4ec94e68, /* ClutCbYCrY R=190, G= 22, B= 22, mix=63       */
    0x52c6527a, /* ClutCbYCrY R=190, G= 22, B= 64, mix=63       */
    0x56c3568c, /* ClutCbYCrY R=190, G= 22, B=106, mix=63       */
    0x5ac05a9e, /* ClutCbYCrY R=190, G= 22, B=148, mix=63       */
    0x5ebd5eb0, /* ClutCbYCrY R=190, G= 22, B=190, mix=63       */
    0x62ba62c3, /* ClutCbYCrY R=190, G= 22, B=232, mix=63       */
    0x63ba635b, /* ClutCbYCrY R=190, G= 64, B= 22, mix=63       */
    0x67b7676e, /* ClutCbYCrY R=190, G= 64, B= 64, mix=63       */
    0x6bb46b80, /* ClutCbYCrY R=190, G= 64, B=106, mix=63       */
    0x6fb16f92, /* ClutCbYCrY R=190, G= 64, B=148, mix=63       */
    0x73ae73a4, /* ClutCbYCrY R=190, G= 64, B=190, mix=63       */
    0x77ab77b7, /* ClutCbYCrY R=190, G= 64, B=232, mix=63       */
    0x78aa784f, /* ClutCbYCrY R=190, G=106, B= 22, mix=63       */
    0x7ca77c62, /* ClutCbYCrY R=190, G=106, B= 64, mix=63       */
    0x80a48074, /* ClutCbYCrY R=190, G=106, B=106, mix=63       */
    0x84a18486, /* ClutCbYCrY R=190, G=106, B=148, mix=63       */
    0x889e8898, /* ClutCbYCrY R=190, G=106, B=190, mix=63       */
    0x8c9b8caa, /* ClutCbYCrY R=190, G=106, B=232, mix=63       */
    0x8d9b8d43, /* ClutCbYCrY R=190, G=148, B= 22, mix=63       */
    0x91989155, /* ClutCbYCrY R=190, G=148, B= 64, mix=63       */
    0x95959568, /* ClutCbYCrY R=190, G=148, B=106, mix=63       */
    0x9992997a, /* ClutCbYCrY R=190, G=148, B=148, mix=63       */
    0x9e8f9e8c, /* ClutCbYCrY R=190, G=148, B=190, mix=63       */
    0xa28ca29e, /* ClutCbYCrY R=190, G=148, B=232, mix=63       */
    0xa28ba237, /* ClutCbYCrY R=190, G=190, B= 22, mix=63       */
    0xa688a649, /* ClutCbYCrY R=190, G=190, B= 64, mix=63       */
    0xaa85aa5c, /* ClutCbYCrY R=190, G=190, B=106, mix=63       */
    0xaf82af6e, /* ClutCbYCrY R=190, G=190, B=148, mix=63       */
    0xb380b380, /* ClutCbYCrY R=190, G=190, B=190, mix=63       */
    0xb77eb792, /* ClutCbYCrY R=190, G=190, B=232, mix=63       */
    0xb77db72b, /* ClutCbYCrY R=190, G=232, B= 22, mix=63       */
    0xbc7abc3d, /* ClutCbYCrY R=190, G=232, B= 64, mix=63       */
    0xc077c04f, /* ClutCbYCrY R=190, G=232, B=106, mix=63       */
    0xc474c462, /* ClutCbYCrY R=190, G=232, B=148, mix=63       */
    0xc871c874, /* ClutCbYCrY R=190, G=232, B=190, mix=63       */
    0xcc6ecc86, /* ClutCbYCrY R=190, G=232, B=232, mix=63       */
    0x58dc5861, /* ClutCbYCrY R=232, G= 22, B= 22, mix=63       */
    0x5cd95c74, /* ClutCbYCrY R=232, G= 22, B= 64, mix=63       */
    0x61d66185, /* ClutCbYCrY R=232, G= 22, B=106, mix=63       */
    0x65d36598, /* ClutCbYCrY R=232, G= 22, B=148, mix=63       */
    0x69d069aa, /* ClutCbYCrY R=232, G= 22, B=190, mix=63       */
    0x6dcd6dbd, /* ClutCbYCrY R=232, G= 22, B=232, mix=63       */
    0x6ecc6e55, /* ClutCbYCrY R=232, G= 64, B= 22, mix=63       */
    0x72c97268, /* ClutCbYCrY R=232, G= 64, B= 64, mix=63       */
    0x76c6767a, /* ClutCbYCrY R=232, G= 64, B=106, mix=63       */
    0x7ac37a8c, /* ClutCbYCrY R=232, G= 64, B=148, mix=63       */
    0x7ec07e9e, /* ClutCbYCrY R=232, G= 64, B=190, mix=63       */
    0x82bd82b0, /* ClutCbYCrY R=232, G= 64, B=232, mix=63       */
    0x83bd8349, /* ClutCbYCrY R=232, G=106, B= 22, mix=63       */
    0x87ba875b, /* ClutCbYCrY R=232, G=106, B= 64, mix=63       */
    0x8bb78b6e, /* ClutCbYCrY R=232, G=106, B=106, mix=63       */
    0x8fb48f80, /* ClutCbYCrY R=232, G=106, B=148, mix=63       */
    0x93b19392, /* ClutCbYCrY R=232, G=106, B=190, mix=63       */
    0x97ae97a4, /* ClutCbYCrY R=232, G=106, B=232, mix=63       */
    0x98ad983d, /* ClutCbYCrY R=232, G=148, B= 22, mix=63       */
    0x9caa9c4f, /* ClutCbYCrY R=232, G=148, B= 64, mix=63       */
    0xa0a7a062, /* ClutCbYCrY R=232, G=148, B=106, mix=63       */
    0xa4a4a474, /* ClutCbYCrY R=232, G=148, B=148, mix=63       */
    0xa8a1a886, /* ClutCbYCrY R=232, G=148, B=190, mix=63       */
    0xac9eac98, /* ClutCbYCrY R=232, G=148, B=232, mix=63       */
    0xad9ead31, /* ClutCbYCrY R=232, G=190, B= 22, mix=63       */
    0xb19bb143, /* ClutCbYCrY R=232, G=190, B= 64, mix=63       */
    0xb598b555, /* ClutCbYCrY R=232, G=190, B=106, mix=63       */
    0xb995b968, /* ClutCbYCrY R=232, G=190, B=148, mix=63       */
    0xbe92be7a, /* ClutCbYCrY R=232, G=190, B=190, mix=63       */
    0xc28fc28c, /* ClutCbYCrY R=232, G=190, B=232, mix=63       */
    0xc28ec224, /* ClutCbYCrY R=232, G=232, B= 22, mix=63       */
    0xc68bc637, /* ClutCbYCrY R=232, G=232, B= 64, mix=63       */
    0xca88ca49, /* ClutCbYCrY R=232, G=232, B=106, mix=63       */
    0xcf85cf5c, /* ClutCbYCrY R=232, G=232, B=148, mix=63       */
    0xd382d36e, /* ClutCbYCrY R=232, G=232, B=190, mix=63       */
    0xd780d780  /* ClutCbYCrY R=232, G=232, B=232, mix=63       */
};
/****************************************************************************/


/* --- enumerations ------------------------------------------------------- */

/* --- prototypes of functions -------------------------------------------- */

/* ------------------------------- End of file ---------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __OSDMAC_H */
