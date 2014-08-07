#ifndef __ASM_SH_IO_H
#define __ASM_SH_IO_H

/*
 * Convention:
 *    read{b,w,l}/write{b,w,l} are for PCI,
 *    while in{b,w,l}/out{b,w,l} are for ISA
 * These may (will) be platform specific function.
 * In addition we have 'pausing' versions: in{b,w,l}_p/out{b,w,l}_p
 * and 'string' versions: ins{b,w,l}/outs{b,w,l}
 * For read{b,w,l} and write{b,w,l} there are also __raw versions, which
 * do not have a memory barrier after them.
 *
 * In addition, we have
 *   ctrl_in{b,w,l}/ctrl_out{b,w,l} for SuperH specific I/O.
 *   which are processor specific.
 */

/*
 * We follow the Alpha convention here:
 *  __inb expands to an inline function call (which either calls via the
 *        mach_vec if generic, or a machine specific implementation)
 *  _inb  is a real function call (note ___raw fns are _ version of __raw)
 *  inb   by default expands to _inb, but the machine specific code may
 *        define it to __inb if it chooses.
 */

#include "common.h"

#include "asm/cache.h"
#include "asm/system.h"
#include "linux/config.h"

/*
 * Depending on which platform we are running on, we need different
 * I/O functions.
 */

/* Control operations through platform specific headers */

#ifdef __KERNEL__

#define _readb(addr) (*(volatile unsigned char *)(addr))
#define _readw(addr) (*(volatile unsigned short *)(addr))
#define _readl(addr) (*(volatile unsigned int *)(addr))

#define _writeb(b,addr) (*(volatile unsigned char *)(addr) = (b))
#define _writew(b,addr) (*(volatile unsigned short *)(addr) = (b))
#define _writel(b,addr) (*(volatile unsigned int *)(addr) = (b))

#define __WANT_IO_DEF

#if defined(CONFIG_SH_MB411)		|| \
       defined(CONFIG_SH_MB442)		|| \
       defined(CONFIG_SH_HMP7100)	|| \
       defined(CONFIG_SH_MB448)		|| \
       defined(CONFIG_SH_HMS1)		|| \
       defined(CONFIG_SH_MB519)		|| \
       defined(CONFIG_SH_MB618)		|| \
       defined(CONFIG_SH_HDK7111)	|| \
       defined(CONFIG_SH_MB628)		|| \
       defined(CONFIG_SH_EUD7141)	|| \
       defined(CONFIG_SH_MB671)		|| \
       defined(CONFIG_SH_MB680)		|| \
       defined(CONFIG_SH_PDK7105)	|| \
       defined(CONFIG_SH_HDK7106)	|| \
       defined(CONFIG_SH_IPIDTV7105)	|| \
       defined(CONFIG_SH_MB704)		|| \
       defined(CONFIG_SH_5197CAB)	|| \
       defined(CONFIG_SH_CB101)		|| \
       defined(CONFIG_SH_CB102)		|| \
       defined(CONFIG_SH_MB837)		|| \
       defined(CONFIG_SH_HDK7108)	|| \
       defined(CONFIG_SH_IDL4K)		|| \
       defined(CONFIG_SH_IDL52K)	|| \
       defined(CONFIG_SH_IDL53K)	|| \
       defined(CONFIG_SH_MB903)		|| \
       defined(CONFIG_SH_B2037)		|| \
       defined(CONFIG_SH_FLI7510)	|| \
       defined(CONFIG_SH_FLI7540)	|| \
       defined(CONFIG_SH_MB796)		|| \
       defined(CONFIG_SH_HDK5289)
#  include "asm/io_stb1eval.h"
#else
#  error "What system is this?"
#endif

#undef __WANT_IO_DEF

#endif /* __KERNEL__ */

/* These are always function calls, in both kernel and user space */

static __inline__ unsigned char _inb (unsigned long addr)
{
	return *(volatile unsigned char *) addr;
}

static __inline__ unsigned short _inw (unsigned long addr)
{
	return *(volatile unsigned short *) addr;
}

static __inline__ unsigned int _inl (unsigned long addr)
{
	return *(volatile unsigned long *) addr;
}

static __inline__ void _outb (unsigned char b, unsigned long addr)
{
	*(volatile unsigned char *) addr = b;
}

static __inline__ void _outw (unsigned short b, unsigned long addr)
{
	*(volatile unsigned short *) addr = b;
}

static __inline__ void _outl (unsigned int b, unsigned long addr)
{
	*(volatile unsigned long *) addr = b;
}

extern void _insb (unsigned long port, void *dst, unsigned long count);
extern void _insw (unsigned long port, void *dst, unsigned long count);
extern void _insl (unsigned long port, void *dst, unsigned long count);
extern void _outsb (unsigned long port, const void *src, unsigned long count);
extern void _outsw (unsigned long port, const void *src, unsigned long count);
extern void _outsl (unsigned long port, const void *src, unsigned long count);

#ifdef __KERNEL__
extern unsigned char ___raw_readb (unsigned long addr);
extern unsigned short ___raw_readw (unsigned long addr);
extern unsigned int ___raw_readl (unsigned long addr);
extern void ___raw_writeb (unsigned char b, unsigned long addr);
extern void ___raw_writew (unsigned short b, unsigned long addr);
extern void ___raw_writel (unsigned int b, unsigned long addr);
#endif

#ifdef __KERNEL__
/*
 * The platform header files may define some of these macros to use
 * the inlined versions where appropriate.  These macros may also be
 * redefined by userlevel programs.
 */
#ifndef inb
# define inb(p)		_inb(p)
#endif
#ifndef inw
# define inw(p)		_inw(p)
#endif
#ifndef inl
# define inl(p)		_inl(p)
#endif

#ifndef outb
# define outb(b,p)	_outb((b),(p))
#endif
#ifndef outw
# define outw(w,p)	_outw((w),(p))
#endif
#ifndef outl
# define outl(l,p)	_outl((l),(p))
#endif

#ifndef inb_p
# define inb_p		_inb_p
#endif
#ifndef inw_p
# define inw_p		_inw_p
#endif
#ifndef inl_p
# define inl_p		_inl_p
#endif

#ifndef outb_p
# define outb_p		_outb_p
#endif
#ifndef outw_p
# define outw_p		_outw_p
#endif
#ifndef outl_p
# define outl_p		_outl_p
#endif

#ifndef insb
# define insb(p,d,c)	_insb((p),(d),(c))
#endif
#ifndef insw
# define insw(p,d,c)	_insw((p),(d),(c))
#endif
#ifndef insl
# define insl(p,d,c)	_insl((p),(d),(c))
#endif
#ifndef outsb
# define outsb(p,s,c)	_outsb((p),(s),(c))
#endif
#ifndef outsw
# define outsw(p,s,c)	_outsw((p),(s),(c))
#endif
#ifndef outsl
# define outsl(p,s,c)	_outsl((p),(s),(c))
#endif

#ifdef __raw_readb
# define readb(a)	({ unsigned long r_ = __raw_readb(a); mb(); r_; })
#endif
#ifdef __raw_readw
# define readw(a)	({ unsigned long r_ = __raw_readw(a); mb(); r_; })
#endif
#ifdef __raw_readl
# define readl(a)	({ unsigned long r_ = __raw_readl(a); mb(); r_; })
#endif

#ifdef __raw_writeb
# define writeb(v,a)	({ __raw_writeb((v),(a)); mb(); })
#endif
#ifdef __raw_writew
# define writew(v,a)	({ __raw_writew((v),(a)); mb(); })
#endif
#ifdef __raw_writel
# define writel(v,a)	({ __raw_writel((v),(a)); mb(); })
#endif

#ifndef __raw_readb
# define __raw_readb(a)	___raw_readb((unsigned long)(a))
#endif
#ifndef __raw_readw
# define __raw_readw(a)	___raw_readw((unsigned long)(a))
#endif
#ifndef __raw_readl
# define __raw_readl(a)	___raw_readl((unsigned long)(a))
#endif

#ifndef __raw_writeb
# define __raw_writeb(v,a)  ___raw_writeb((v),(unsigned long)(a))
#endif
#ifndef __raw_writew
# define __raw_writew(v,a)  ___raw_writew((v),(unsigned long)(a))
#endif
#ifndef __raw_writel
# define __raw_writel(v,a)  ___raw_writel((v),(unsigned long)(a))
#endif

#ifndef readb
# define readb(a)	_readb((unsigned long)(a))
#endif
#ifndef readw
# define readw(a)	_readw((unsigned long)(a))
#endif
#ifndef readl
# define readl(a)	_readl((unsigned long)(a))
#endif

#ifndef writeb
# define writeb(v,a)	_writeb((v),(unsigned long)(a))
#endif
#ifndef writew
# define writew(v,a)	_writew((v),(unsigned long)(a))
#endif
#ifndef writel
# define writel(v,a)	_writel((v),(unsigned long)(a))
#endif

#else

/* Userspace declarations.  */

extern unsigned char inb (unsigned long port);
extern unsigned short inw (unsigned long port);
extern unsigned int inl (unsigned long port);
extern void outb (unsigned char b, unsigned long port);
extern void outw (unsigned short w, unsigned long port);
extern void outl (unsigned int l, unsigned long port);
extern void insb (unsigned long port, void *dst, unsigned long count);
extern void insw (unsigned long port, void *dst, unsigned long count);
extern void insl (unsigned long port, void *dst, unsigned long count);
extern void outsb (unsigned long port, const void *src, unsigned long count);
extern void outsw (unsigned long port, const void *src, unsigned long count);
extern void outsl (unsigned long port, const void *src, unsigned long count);
extern unsigned char readb (unsigned long addr);
extern unsigned short readw (unsigned long addr);
extern unsigned long readl (unsigned long addr);
extern void writeb (unsigned char b, unsigned long addr);
extern void writew (unsigned short b, unsigned long addr);
extern void writel (unsigned int b, unsigned long addr);

#endif /* __KERNEL__ */

#ifdef __KERNEL__

/*
 * If the platform has PC-like I/O, this function converts the offset into
 * an address.
 */
static __inline__ unsigned long isa_port2addr (unsigned long offset)
{
	return __isa_port2addr (offset);
}

#define isa_readb(a) readb(isa_port2addr(a))
#define isa_readw(a) readw(isa_port2addr(a))
#define isa_readl(a) readl(isa_port2addr(a))
#define isa_writeb(b,a) writeb(b,isa_port2addr(a))
#define isa_writew(w,a) writew(w,isa_port2addr(a))
#define isa_writel(l,a) writel(l,isa_port2addr(a))
#define isa_memset_io(a,b,c) \
  memset((void *)(isa_port2addr((unsigned long)a)),(b),(c))
#define isa_memcpy_fromio(a,b,c) \
  memcpy((a),(void *)(isa_port2addr((unsigned long)(b))),(c))
#define isa_memcpy_toio(a,b,c) \
  memcpy((void *)(isa_port2addr((unsigned long)(a))),(b),(c))

/* We really want to try and get these to memcpy etc */
extern void memcpy_fromio (void *, unsigned long, unsigned long);
extern void memcpy_toio (unsigned long, const void *, unsigned long);
extern void memset_io (unsigned long, int, unsigned long);

static __inline__ unsigned char ctrl_inb (unsigned long addr)
{
	return *(volatile unsigned char *) addr;
}

static __inline__ unsigned short ctrl_inw (unsigned long addr)
{
	return *(volatile unsigned short *) addr;
}

static __inline__ unsigned int ctrl_inl (unsigned long addr)
{
	return *(volatile unsigned long *) addr;
}

static __inline__ void ctrl_outb (unsigned char b, unsigned long addr)
{
	*(volatile unsigned char *) addr = b;
}

static __inline__ void ctrl_outw (unsigned short b, unsigned long addr)
{
	*(volatile unsigned short *) addr = b;
}

static __inline__ void ctrl_outl (unsigned int b, unsigned long addr)
{
	*(volatile unsigned long *) addr = b;
}


#define IO_SPACE_LIMIT 0xffffffff

/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the x86 architecture, we just read/write the
 * memory location directly.
 *
 * On SH, we have the whole physical address space mapped at all times
 * (as MIPS does), so "ioremap()" and "iounmap()" do not need to do
 * anything.  (This isn't true for all machines but we still handle
 * these cases with wired TLB entries anyway ...)
 *
 * We cheat a bit and always return uncachable areas until we've fixed
 * the drivers to handle caching properly.
 */
static __inline__ void *ioremap (unsigned long offset, unsigned long size)
{
	return __ioremap (offset, size);
}

static __inline__ void iounmap (void *addr)
{
	return __iounmap (addr);
}

#define ioremap_nocache(off,size) ioremap(off,size)

static __inline__ int check_signature (unsigned long io_addr,
				       const unsigned char *signature,
				       int length)
{
	int retval = 0;
	do {
		if (readb (io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
      out:
	return retval;
}

/*
 * The caches on some architectures aren't dma-coherent and have need to
 * handle this in software.  There are three types of operations that
 * can be applied to dma buffers.
 *
 *  - dma_cache_wback_inv(start, size) makes caches and RAM coherent by
 *    writing the content of the caches back to memory, if necessary.
 *    The function also invalidates the affected part of the caches as
 *    necessary before DMA transfers from outside to memory.
 *  - dma_cache_inv(start, size) invalidates the affected parts of the
 *    caches.  Dirty lines of the caches may be written back or simply
 *    be discarded.  This operation is necessary before dma operations
 *    to the memory.
 *  - dma_cache_wback(start, size) writes back any dirty lines but does
 *    not invalidate the cache.  This can be used before DMA reads from
 *    memory,
 */

#define dma_cache_wback_inv(_start,_size) \
    __flush_purge_region(_start,_size)
#define dma_cache_inv(_start,_size) \
    __flush_invalidate_region(_start,_size)
#define dma_cache_wback(_start,_size) \
    __flush_wback_region(_start,_size)

static inline void sync(void)
{
	/* do nothing */
	/* Note: may need to include a "synco" instruction here, if we
	 * have silicon with agressive write-combiners on the SH4-300
	 * series cores in the future */
}

#endif /* __KERNEL__ */
#endif /* __ASM_SH_IO_H */
