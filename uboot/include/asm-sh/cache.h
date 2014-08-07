/*
 * include/asm-sh/cache.h
 *
 * Copyright 1999 (C) Niibe Yutaka
 * Copyright 2005 (C) Andy Sturges
 */

#ifndef __ASM_SH_CACHE_H
#define __ASM_SH_CACHE_H

#define L1_CACHE_BYTES			32

#define CACHE_IC_ADDRESS_ARRAY		0xf0000000
#define CACHE_OC_ADDRESS_ARRAY		0xf4000000

#if defined(CONFIG_CPU_SUBTYPE_SH4_1XX)		/* it's an SH4-100 */
#	define DCACHE_WAY_INCR		(1 << 14)
#	define DCACHE_ENTRY_SHIFT	5
#	define DCACHE_ENTRY_MASK	0x3fe0
#	define DCACHE_SETS		512
#	define DCACHE_WAYS		1
#	define DCACHE_LINESZ		L1_CACHE_BYTES
#elif defined(CONFIG_CPU_SUBTYPE_SH4_2XX)	/* it's an SH4-200 */	\
   || defined(CONFIG_CPU_SUBTYPE_SH4_3XX)	/* it's an SH4-300 */
#	define DCACHE_SIZE		32768
#	define DCACHE_WAY_INCR		(DCACHE_SIZE >> 1)
#	define DCACHE_ENTRY_SHIFT	5
#	define DCACHE_ENTRY_MASK	(DCACHE_WAY_INCR - (1 << 5))
#	define DCACHE_SETS		(DCACHE_SIZE >> 6)
#	define DCACHE_WAYS		2
#	define DCACHE_LINESZ		L1_CACHE_BYTES
#else
#	error Unknown CPU
#endif


#ifndef __ASSEMBLER__

extern void sh_cache_set_op(unsigned long);
extern void sh_cache_clear_op(unsigned long);

extern void sh_flush_cache_all(void);
extern void sh_disable_data_caches(void);
extern void sh_enable_data_caches(void);

/* following stores if the data caches currently on */
extern int sh_data_caches_on;

#endif	/* __ASSEMBLER__ */

#endif /* __ASM_SH_CACHE_H */
