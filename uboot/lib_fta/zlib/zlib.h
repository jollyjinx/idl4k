#ifndef ZLIB_H
#define ZLIB_H


#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_PARAM    __attribute__((unused))
#include "fta/types.h"

/* ========================================================================
   versioning
   ======================================================================== */
#define ZLIB_VERSION "1.2.3"
#define ZLIB_VERNUM 0x1230

/* ========================================================================
   top level definitions
   ======================================================================== */
#define GUNZIP      /* define this for support of GUNZIP                 */
#define UNCOMPRESS  /* define this if you want support for uncompression */


/* ========================================================================
   some global definitions
   ======================================================================== */
#define MAX_WBITS       15 /* 32K LZ77 window */
#define DEF_MEM_LEVEL   8  /* memory level    */

/* ========================================================================
   diagnostic functions
   ======================================================================== */
#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)
#define z_verbose   1
/* ========================================================================
   macro redefintions
   ======================================================================== */
#define zmemcpy memcpy
#define zmemcmp memcmp
#define zmemzero(dest, len) memset(dest, 0, len)
#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]
#define ERR_RETURN(strm,err) return (strm->msg = (char*)ERR_MSG(err), (err) )
#define ZALLOC(strm, items, size)  (*((strm)->zalloc) )((strm)->opaque, (items), (size) )
#define ZFREE(strm, addr)          (*( (strm)->zfree) )( (strm)->opaque, (void *)(addr) )
#define TRY_FREE(s, p)             {if (p) ZFREE(s, p);}

typedef void * (*alloc_func) (void * opaque, unsigned int items, unsigned int size);
typedef void   (*free_func)  (void * opaque, void * address);
struct internal_state;
typedef struct z_stream_s 
{
    unsigned char          *next_in;  /* next input byte */
    unsigned int           avail_in;  /* number of bytes available at next_in */
    unsigned long          total_in;  /* total nb of input bytes read so far */
    unsigned char          *next_out; /* next output byte should be put there */
    unsigned int           avail_out; /* remaining free space at next_out */
    unsigned long          total_out; /* total nb of bytes output so far */
    char                   *msg;      /* last error message, NULL if no error */
    struct internal_state  *state;    /* not visible by applications */
    alloc_func             zalloc;    /* used to allocate the internal state */
    free_func              zfree;     /* used to free the internal state */
    void *                 opaque;    /* private data object passed to zalloc and zfree */
    int                    data_type; /* best guess about the data type: binary or text */
    unsigned long          adler;     /* adler32 value of the uncompressed data */
    unsigned long          reserved;  /* reserved for future use */
}   z_stream;

typedef struct gz_header_s 
{
    int             text;       /* true if compressed data believed to be text */
    unsigned long   time;       /* modification time */
    int             xflags;     /* extra flags (not used when writing a gzip file) */
    int             os;         /* operating system */
    unsigned char   *extra;     /* pointer to extra field or Z_NULL if none */
    unsigned int    extra_len;  /* extra field length (valid if extra != Z_NULL) */
    unsigned int    extra_max;  /* space at extra (only when reading header) */
    unsigned char   *name;      /* pointer to zero-terminated file name or Z_NULL */
    unsigned int    name_max;   /* space at name (only when reading header) */
    unsigned char   *comment;   /* pointer to zero-terminated comment or Z_NULL */
    unsigned int    comm_max;   /* space at comment (only when reading header) */
    int             hcrc;       /* true if there was or will be a header crc */
    int             done;       /* true when done reading gzip header (not used when writing a gzip file) */
} 	gz_header;
/* ========================================================================
   constants
   ======================================================================== */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
#define Z_BLOCK         5
/* ========================================================================
   returns values
   ======================================================================== */
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* ========================================================================
   compression decompression codes
   ======================================================================== */
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* ========================================================================
   compression levels
   ======================================================================== */
#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_RLE                 3
#define Z_FIXED               4
#define Z_DEFAULT_STRATEGY    0
/* ========================================================================
   compression strategy
   ======================================================================== */
#define Z_BINARY     0
#define Z_TEXT       1
#define Z_ASCII      1     /* for compatibility with 1.2.2 and earlier */
#define Z_UNKNOWN    2     /* Possible values of the data_type field (though see inflate() ) */
#define Z_DEFLATED   8     /* The deflate compression method (the only one supported in this version) */
#define Z_NULL       0     /* for initializing zalloc, zfree, opaque */
/* ========================================================================
   utilities and usefull functions
   ======================================================================== */
extern const char    * const z_errmsg[10];
extern void          * zcalloc              (void * opaque, unsigned items, unsigned size);
extern void            zcfree               (void * opaque, void * ptr);
unsigned long          adler32f             (unsigned long adler, const unsigned char *buf, unsigned int len);
extern unsigned long   crc32                (unsigned long crc, const unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* ZLIB_H */


