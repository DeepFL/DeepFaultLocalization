#if !defined(INTEGER_TYPES_H)
#define INTEGER_TYPES_H

/*                                                               */
/* Copyright 1984,1985,1986,1988,1989,1990,2003,2004,2005,       */
/*   2006 by Howard Turtle                                       */
/*                                                               */

#ifdef WIN32
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef unsigned __int64   UINT64;
#define UINT64_format "%llu"
#define UINT64_formatf(w) "%" #w "llu"
#define UINT64_C(c)   c ## ULL
#define PATH_SEPARATOR '\\'
typedef struct F_HANDLE F_HANDLE;
#else
#include <inttypes.h>
typedef uint16_t           UINT16;
typedef uint32_t           UINT32;
typedef uint64_t           UINT64;
#define UINT64_format "%" PRIu64
#define UINT64_formatf(w) "%" #w PRIu64
#define PATH_SEPARATOR '/'
#define F_HANDLE FILE
#endif


#endif
