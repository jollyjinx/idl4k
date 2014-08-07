/*****************************************************************************

File name   :  inflate.h

Description :  

COPYRIGHT (C) Inverto Digital Labs 2009.

Date               Modification                                          Name
----               ------------                                          ----
 02/03/10          Created                                               FR

*****************************************************************************/
/* --- prevents recursive inclusion --------------------------------------- */
#ifndef __inflate_H
#define __inflate_H

/* --- allows C compiling with C++ compiler ------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ----------------------------------------------------------- */
#include "zlib.h"

/* --- defines ------------------------------------------------------------ */
#define ENOUGH  2048
#define MAXD    592
#define MAXBITS 15

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#define UPDATE(check, buf, len) (state->flags ? crc32(check, buf, len) : adler32f(check, buf, len))
#else
#define UPDATE(check, buf, len) adler32f(check, buf, len)
#endif
/* check macros for header crc */
#define CRC2(check, word) { hbuf[0] = (unsigned char)(word); hbuf[1] = (unsigned char)((word) >> 8); check = crc32(check, hbuf, 2); }
#define CRC4(check, word) { hbuf[0] = (unsigned char)(word); hbuf[1] = (unsigned char)((word) >> 8); hbuf[2] = (unsigned char)((word) >> 16); hbuf[3] = (unsigned char)((word) >> 24); check = crc32(check, hbuf, 4); }
/* Load registers with state in inflate() for speed */
#define LOAD() { put = strm->next_out; left = strm->avail_out; next = strm->next_in; have = strm->avail_in; hold = state->hold; bits = state->bits; }
/* Restore state from registers in inflate() */
#define RESTORE() { strm->next_out = put; strm->avail_out = left; strm->next_in = next; strm->avail_in = have; state->hold = hold; state->bits = bits; }
/* Clear the input bit accumulator */
#define INITBITS() { hold = 0; bits = 0; }
/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() { if (have == 0) goto inf_leave; have--; hold += (unsigned long)(*next++) << bits; bits += 8; }
/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) { while (bits < (unsigned)(n)) PULLBYTE(); }
/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) ((unsigned)hold & ((1U << (n)) - 1))
/* Remove n bits from the bit accumulator */
#define DROPBITS(n) {   hold >>= (n); bits -= (unsigned)(n); }
/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() { hold >>= bits & 7; bits -= bits & 7; }
/* Reverse the bytes in a 32-bit value */
#define REVERSE(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

/* --- enumerations ------------------------------------------------------- */
/* Type of code to build for inftable() */
typedef enum 
{
    CODES,
    LENS,
    DISTS
} codetype;

/* Possible inflate modes between inflate() calls */
typedef enum 
{
    HEAD,       /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    TIME,       /* i: waiting for modification time (gzip) */
    OS,         /* i: waiting for extra flags and operating system (gzip) */
    EXLEN,      /* i: waiting for extra length (gzip) */
    EXTRA,      /* i: waiting for extra bytes (gzip) */
    NAME,       /* i: waiting for end of file name (gzip) */
    COMMENT,    /* i: waiting for end of comment (gzip) */
    HCRC,       /* i: waiting for header crc (gzip) */
    DICTID,     /* i: waiting for dictionary check value */
    DICT,       /* waiting for inflateSet Dictionary() call */
    TYPE,       /* i: waiting for type bits, including last-flag bit */
    TYPEDO,     /* i: same, but skip check to exit inflate on new block */
    STORED,     /* i: waiting for stored size (length and complement) */
    COPY,       /* i/o: waiting for input or output to copy stored block */
    TABLE,      /* i: waiting for dynamic block table lengths */
    LENLENS,    /* i: waiting for code length code lengths */
    CODELENS,   /* i: waiting for length/lit and distance code lengths */
    LEN,        /* i: waiting for length/lit code */
    LENEXT,     /* i: waiting for length extra bits */
    DIST,       /* i: waiting for distance code */
    DISTEXT,    /* i: waiting for distance extra bits */
    MATCH,      /* o: waiting for output space to copy string */
    LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    LENGTH,     /* i: waiting for 32-bit length (gzip) */
    DONE,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_mode;

/* --- variables ---------------------------------------------------------- */
typedef struct 
{
    unsigned char op;           /* operation, extra bits, table bits */
    unsigned char bits;         /* bits in this part of the code */
    unsigned short val;         /* offset in table or code value */
}   code;

/* state maintained between inflate() calls.  Approximately 7K bytes. */
struct inflate_state 
{
    inflate_mode mode;          /* current inflate mode */
    int last;                   /* true if processing last block */
    int wrap;                   /* bit 0 true for zlib, bit 1 true for gzip */
    int havedict;               /* true if dictionary provided */
    int flags;                  /* gzip header method and flags (0 if zlib) */
    unsigned dmax;              /* zlib header max distance (INFLATE_STRICT) */
    unsigned long check;        /* protected copy of check value */
    unsigned long total;        /* protected copy of output count */
    gz_header * head;           /* where to save gzip header information sliding window */
    unsigned wbits;             /* log base 2 of requested window size */
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned write;             /* window write index */
    unsigned char  *window;     /* allocated sliding window, if needed */
    unsigned long hold;         /* bit accumulator: input bit accumulator */
    unsigned bits;              /* bit accumulator: number of bits in "in" */
    unsigned length;            /* literal or length of data to copy */
    unsigned offset;            /* distance back to copy string from */
    unsigned extra;             /* extra bits needed */
    code const  *lencode;       /* starting table for length/literal codes */
    code const  *distcode;      /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    unsigned ncode;             /* number of code length code lengths */
    unsigned nlen;              /* number of length code lengths */
    unsigned ndist;             /* number of distance code lengths */
    unsigned have;              /* number of code lengths in lens[] */
    code  *next;                /* next available space in codes[] */
    unsigned short lens[320];   /* temporary storage for code lengths */
    unsigned short work[288];   /* work area for code table building */
    code codes[ENOUGH];         /* space for code tables */
};

/* ------------------------------- End of file ---------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __inflate_H */











