//=============================================================================
//  Audio Utility Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __DSP_H__
#define __DSP_H__

namespace Ms {

//---------------------------------------------------------
//   f_max
//---------------------------------------------------------

static inline float f_max(float x, float a)
      {
      x -= a;
      x += fabsf(x);
      x *= 0.5f;
      x += a;
      return x;
      }

//---------------------------------------------------------
//   Dsp
//    standard version of all dsp routines without any
//    hw acceleration
//---------------------------------------------------------

class Dsp {
   public:
      Dsp() {}
      virtual ~Dsp() {}

      virtual float peak(float* buf, unsigned n, float current) {
            for (unsigned i = 0; i < n; ++i)
                  current = f_max(current, fabsf(buf[i]));
            return current;
            }
      virtual void applyGainToBuffer(float* buf, unsigned n, float gain) {
            for (unsigned i = 0; i < n; ++i)
                  buf[i] *= gain;
            }
      virtual void mixWithGain(float* dst, float* src, unsigned n, float gain) {
            for (unsigned i = 0; i < n; ++i)
                  dst[i] += src[i] * gain;
            }
      virtual void mix(float* dst, float* src, unsigned n) {
            for (unsigned i = 0; i < n; ++i)
                  dst[i] += src[i];
            }
      virtual void cpy(float* dst, float* src, unsigned n) {
#if defined(ARCH_X86) || defined(ARCH_X86_64)
            register unsigned long int dummy;
            __asm__ __volatile__ ("rep; movsl" :"=&D"(dst), "=&S"(src), "=&c"(dummy) :"0" (to), "1" (from),"2" (n) : "memory");
#else
            memcpy(dst, src, sizeof(float) * n);
#endif
            }
      };

extern void initDsp();
extern Dsp* dsp;


}     // namespace Ms
#endif

