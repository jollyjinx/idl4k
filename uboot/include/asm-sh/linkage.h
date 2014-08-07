#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#define SYMBOL_NAME(name) name
#define SYMBOL_NAME_LABEL(name) name ## :

#define ALIGN .balign 4

#define ENTRY(name) \
  .globl SYMBOL_NAME(name); \
  ALIGN; \
  SYMBOL_NAME_LABEL(name)

#endif
