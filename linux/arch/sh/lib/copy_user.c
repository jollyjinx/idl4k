#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/page.h>

__kernel_size_t __copy_user(void *to, const void *from, __kernel_size_t n)
{
    if (!to)
        return n;

   memcpy(to, from, n);
   return 0;
}
