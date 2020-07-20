#ifndef _TYPES_H
#define _TYPES_H

#ifdef HAVE_SDL
#include <SDL.h>
#else
//#include <stdint.h>
#endif

#include <stdlib.h>

typedef struct _MEM_PATCH {
	unsigned int addr;
	unsigned char byte;
} mem_patch;

#ifdef __GNUC__ /*__GNUC__*/
typedef unsigned long long ClockCycle;
#define TSTATE_T_MID (((long long) -1LL)/2ULL)
#define TSTATE_T_LEN "Lu"
#else
typedef unsigned __int64 ClockCycle;
#define TSTATE_T_MID (((__int64) -1L)/2UL)
#define TSTATE_T_LEN "lu"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if ( defined(_MSC_VER) && (_MSC_VER == 1200) && defined(_MSC_EXTENSIONS) )
#define inline __forceinline  // For MSVC++ 6.0 with Microsoft extensions.
#endif

#endif // _TYPES_H
