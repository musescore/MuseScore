/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
    Copyright (C) 2008 Hans Fugal <hans@fugal.net> (OSX version)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __GLOBAL_H
#define __GLOBAL_H


#ifdef __APPLE__
#include <machine/endian.h>
#define __LITTLE_ENDIAN	__DARWIN_LITTLE_ENDIAN
#define __BIG_ENDIAN	__DARWIN_BIG_ENDIAN
#define __PDP_ENDIAN	__DARWIN_PDP_ENDIAN
#define	__BYTE_ORDER	__DARWIN_BYTE_ORDER
#else
#ifdef __MINGW32__
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#include <endian.h>
#endif
#endif

#ifdef __BYTE_ORDER
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define WR2(p,v) { (p)[0] = v; (p)[1] = v >> 8; }
#define WR4(p,v) { (p)[0] = v; (p)[1] = v >> 8;  (p)[2] = v >> 16;  (p)[3] = v >> 24; }
#define RD2(p) ((p)[0] + ((p)[1] << 8));
#define RD4(p) ((p)[0] + ((p)[1] << 8) + ((p)[2] << 16) + ((p)[3] << 24));
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#define WR2(p,v) { (p)[1] = v; (p)[0] = v >> 8; }
#define WR4(p,v) { (p)[3] = v; (p)[2] = v >> 8;  (p)[1] = v >> 16;  (p)[0] = v >> 24; }
#define RD2(p) ((p)[1] + ((p)[0] << 8));
#define RD4(p) ((p)[3] + ((p)[2] << 8) + ((p)[1] << 16) + ((p)[0] << 24));
#else
#error Byte order is not supported !
#endif
#else
#error Byte order is undefined !
#endif

enum // GLOBAL LIMITS
{
    NASECT = 4,
    NDIVIS = 8,
    NKEYBD = 6,
    NGROUP = 8,
    NNOTES = 61,
    NBANK  = 32,
    NPRES  = 32
};


#define MIDICTL_SWELL 7
#define SWELL_MIN 0.0f
#define SWELL_MAX 1.0f
#define SWELL_DEF 1.0f

#define MIDICTL_TFREQ 12
#define TFREQ_MIN 2.0f
#define TFREQ_MAX 8.0f
#define TFREQ_DEF 4.0f

#define MIDICTL_TMODD 13
#define TMODD_MIN 0.0f
#define TMODD_MAX 0.6f
#define TMODD_DEF 0.3f

#define MIDICTL_BANK   32
#define MIDICTL_HOLD   64
#define MIDICTL_IFELM  98
#define MIDICTL_ASOFF 120
#define MIDICTL_ANOFF 123

#define KEYS_MASK 63
#define HOLD_MASK 64
#define ALL_MASK 127

#include "synthesizer/synthesizer.h"
#include "synthesizer/midipatch.h"
#include "effects/effect.h"
#include "sparm.h"

#endif

