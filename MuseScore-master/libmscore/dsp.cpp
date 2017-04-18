//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2006 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "dsp.h"

Dsp* dsp;

#ifdef __i386__

//---------------------------------------------------------
//   DspSSE86
//---------------------------------------------------------

extern "C" {
extern float x86_sse_compute_peak(float*, unsigned, float);
extern void x86_sse_apply_gain_to_buffer(float*, unsigned, float);
extern void x86_sse_mix_buffers_with_gain(float*, float*, unsigned, float);
extern void x86_sse_mix_buffers_no_gain(float*, float*, unsigned);
   };

class DspSSE86 : public Dsp {
   public:
      DspSSE86() {}
      virtual ~DspSSE86() {}

      virtual float peak(float* buf, unsigned n, float current) {
            if ( ((intptr_t)buf % 16) != 0) {
                  qDebug("peak(): buffer unaligned! (%p)\n", buf);
                  return Dsp::peak(buf, n, current);
                  }
            return x86_sse_compute_peak(buf, n, current);
            }

      virtual void applyGainToBuffer(float* buf, unsigned n, float gain) {
            if ( ((intptr_t)buf % 16) != 0) {
                  qDebug("applyGainToBuffer(): buffer unaligned! (%p)\n", buf);
                  Dsp::applyGainToBuffer(buf, n, gain);
                  }
            else
                  x86_sse_apply_gain_to_buffer(buf, n, gain);
            }

      virtual void mixWithGain(float* dst, float* src, unsigned n, float gain) {
            if ( ((intptr_t)dst & 15) != 0)
                  qDebug("mixWithGainain(): dst unaligned! (%p)\n", dst);
            if (((intptr_t)dst & 15) != ((intptr_t)src & 15) ) {
                  qDebug("mixWithGain(): dst & src don't have the same alignment!\n");
                  Dsp::mixWithGain(dst, src,n, gain);
                  }
            else
                  x86_sse_mix_buffers_with_gain(dst, src, n, gain);
            }
      virtual void mix(float* dst, float* src, unsigned n) {
            if ( ((intptr_t)dst & 15) != 0)
                  qDebug("mix_buffers_no_gain(): dst unaligned! %p\n", dst);
            if ( ((intptr_t)dst & 15) != ((intptr_t)src & 15) ) {
                  qDebug("mix_buffers_no_gain(): dst & src don't have the same alignment!\n");
                  Dsp::mix(dst, src, n);
                  }
            else
                  x86_sse_mix_buffers_no_gain(dst, src, n);
            }
      };
#endif

//---------------------------------------------------------
//   initDsp
//---------------------------------------------------------

void initDsp()
      {
#if defined(__i386__) && defined(USE_SSE)
      unsigned long useSSE = 0;

#ifdef __x86_64__
      useSSE = 1 << 25;       // we know the platform has SSE
#else
      asm (
         "mov $1, %%eax\n"
         "pushl %%ebx\n"
         "cpuid\n"
         "movl %%edx, %0\n"
         "popl %%ebx\n"
         : "=r" (useSSE)
         :
         : "%eax", "%ecx", "%edx", "memory");
#endif
      useSSE &= (1 << 25); // bit 25 = SSE support
      if (useSSE) {
            qDebug("Using SSE optimized routines\n");
            dsp = new DspSSE86();
            return;
            }
      // fall through to not hardware optimized routines
#endif
      dsp = new Dsp();
      }
