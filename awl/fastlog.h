/* Copyright unknown. Code by Laurent de Soras <laurent@ohmforce.com>.
 * This file is licensed under the WTFPL v2 license.
 * http://www.wtfpl.net/about/
 */

#ifndef __FASTLOG_H__
#define __FASTLOG_H__

#include <math.h> /* for HUGE_VAL */

static inline float fast_log2 (float val)
      {
	/* don't use reinterpret_cast<> because that prevents this
	   from being used by pure C code (for example, GnomeCanvasItems)
	*/
      union {float f; int i;} t;
	t.f = val;
      int* const exp_ptr = &t.i;
	int x              = *exp_ptr;
	const int log_2    = ((x >> 23) & 255) - 128;
	x &= ~(255 << 23);
	x += 127 << 23;
	*exp_ptr = x;
	val = ((-1.0f/3) * t.f + 2) * t.f - 2.0f/3;
	return (val + log_2);
      }

static inline float fast_log (const float val)
      {
      return (fast_log2 (val) * 0.69314718f);
      }

static inline float fast_log10 (const float val)
      {
	return fast_log2(val) / 3.312500f;
      }

static inline float minus_infinity() { return -HUGE_VAL; }

#endif

