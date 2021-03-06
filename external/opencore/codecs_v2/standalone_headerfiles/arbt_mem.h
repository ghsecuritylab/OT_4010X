
#ifndef ARBT_MEM_H_INCLUDED
#define ARBT_MEM_H_INCLUDED

#include "arbt_types.h"

#define ARBTMemSizeT size_t

#define arbt_memcpy(dest, src, count)       memcpy((void *)(dest), (const void *)(src), (ARBTMemSizeT)(count))
#define arbt_memset(dest, ch, count)        memset((void *)(dest), (unsigned char)(ch), (ARBTMemSizeT)(count))
#define arbt_memmove(dest, src, bytecount)  memmove((void *)(dest), (const void *)(src), (ARBTMemSizeT)(bytecount))
#define arbt_memcmp(buf1, buf2, count)      memcmp( (const void *)(buf1), (const void *)(buf2), (ARBTMemSizeT)(count))
#define arbt_malloc(size)                      malloc((ARBTMemSizeT)(size))
#define arbt_free(memblock)                 free((void *)(memblock))
#define ARBT_ARRAY_DELETE(ptr)              delete [] ptr
#define ARBT_ARRAY_NEW(T, count)            new T[count]
#define ARBT_DELETE(memblock)               delete memblock
#define ARBT_NEW(arg)                       new arg

#endif // ARBT_MEM_H_INCLUDED

