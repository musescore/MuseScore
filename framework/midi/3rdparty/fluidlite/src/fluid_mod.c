/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluid_mod.h"
#include "fluid_chan.h"
#include "fluid_voice.h"

/*
 * fluid_mod_clone
 */
void
fluid_mod_clone(fluid_mod_t* mod, fluid_mod_t* src)
{
  mod->dest = src->dest;
  mod->src1 = src->src1;
  mod->flags1 = src->flags1;
  mod->src2 = src->src2;
  mod->flags2 = src->flags2;
  mod->amount = src->amount;
}

/*
 * fluid_mod_set_source1
 */
void
fluid_mod_set_source1(fluid_mod_t* mod, int src, int flags)
{
  mod->src1 = src;
  mod->flags1 = flags;
}

/*
 * fluid_mod_set_source2
 */
void
fluid_mod_set_source2(fluid_mod_t* mod, int src, int flags)
{
  mod->src2 = src;
  mod->flags2 = flags;
}

/*
 * fluid_mod_set_dest
 */
void
fluid_mod_set_dest(fluid_mod_t* mod, int dest)
{
  mod->dest = dest;
}

/*
 * fluid_mod_set_amount
 */
void
fluid_mod_set_amount(fluid_mod_t* mod, double amount)
{
  mod->amount = (double) amount;
}

int fluid_mod_get_source1(fluid_mod_t* mod)
{
  return mod->src1;
}

int fluid_mod_get_flags1(fluid_mod_t* mod)
{
  return mod->flags1;
}

int fluid_mod_get_source2(fluid_mod_t* mod)
{
  return mod->src2;
}

int fluid_mod_get_flags2(fluid_mod_t* mod)
{
  return mod->flags2;
}

int fluid_mod_get_dest(fluid_mod_t* mod)
{
  return mod->dest;
}

double fluid_mod_get_amount(fluid_mod_t* mod)
{
  return (fluid_real_t) mod->amount;
}


/*
 * fluid_mod_get_value
 */
fluid_real_t
fluid_mod_get_value(fluid_mod_t* mod, fluid_channel_t* chan, fluid_voice_t* voice)
{
  fluid_real_t v1 = 0.0, v2 = 1.0;
  fluid_real_t range1 = 127.0, range2 = 127.0;

  if (chan == NULL) {
    return 0.0f;
  }

  /* 'special treatment' for default controller
   *
   *  Reference: SF2.01 section 8.4.2
   *
   * The GM default controller 'vel-to-filter cut off' is not clearly
   * defined: If implemented according to the specs, the filter
   * frequency jumps between vel=63 and vel=64.  To maintain
   * compatibility with existing sound fonts, the implementation is
   * 'hardcoded', it is impossible to implement using only one
   * modulator otherwise.
   *
   * I assume here, that the 'intention' of the paragraph is one
   * octave (1200 cents) filter frequency shift between vel=127 and
   * vel=64.  'amount' is (-2400), at least as long as the controller
   * is set to default.
   *
   * Further, the 'appearance' of the modulator (source enumerator,
   * destination enumerator, flags etc) is different from that
   * described in section 8.4.2, but it matches the definition used in
   * several SF2.1 sound fonts (where it is used only to turn it off).
   * */
  if ((mod->src2 == FLUID_MOD_VELOCITY) &&
      (mod->src1 == FLUID_MOD_VELOCITY) &&
      (mod->flags1 == (FLUID_MOD_GC | FLUID_MOD_UNIPOLAR
		       | FLUID_MOD_NEGATIVE | FLUID_MOD_LINEAR)) &&
      (mod->flags2 == (FLUID_MOD_GC | FLUID_MOD_UNIPOLAR
		       | FLUID_MOD_POSITIVE | FLUID_MOD_SWITCH)) &&
      (mod->dest == GEN_FILTERFC)) {
// S. Christian Collins' mod, to stop forcing velocity based filtering
/*
    if (voice->vel < 64){
      return (fluid_real_t) mod->amount / 2.0;
    } else {
      return (fluid_real_t) mod->amount * (127 - voice->vel) / 127;
    }
*/
     return 0; // (fluid_real_t) mod->amount / 2.0;
  }
// end S. Christian Collins' mod

  /* get the initial value of the first source */
  if (mod->src1 > 0) {
    if (mod->flags1 & FLUID_MOD_CC) {
      v1 = fluid_channel_get_cc(chan, mod->src1);
    } else {  /* source 1 is one of the direct controllers */
      switch (mod->src1) {
      case FLUID_MOD_NONE:         /* SF 2.01 8.2.1 item 0: src enum=0 => value is 1 */
	v1 = range1;
	break;
      case FLUID_MOD_VELOCITY:
	v1 = voice->vel;
	break;
      case FLUID_MOD_KEY:
	v1 = voice->key;
	break;
      case FLUID_MOD_KEYPRESSURE:
	v1 = chan->key_pressure;
	break;
      case FLUID_MOD_CHANNELPRESSURE:
	v1 = chan->channel_pressure;
	break;
      case FLUID_MOD_PITCHWHEEL:
	v1 = chan->pitch_bend;
	range1 = 0x4000;
	break;
      case FLUID_MOD_PITCHWHEELSENS:
	v1 = chan->pitch_wheel_sensitivity;
	break;
      default:
	v1 = 0.0;
      }
    }

    /* transform the input value */
    switch (mod->flags1 & 0x0f) {
    case 0: /* linear, unipolar, positive */
      v1 /= range1;
      break;
    case 1: /* linear, unipolar, negative */
      v1 = 1.0f - v1 / range1;
      break;
    case 2: /* linear, bipolar, positive */
      v1 = -1.0f + 2.0f * v1 / range1;
      break;
    case 3: /* linear, bipolar, negative */
      v1 = 1.0f - 2.0f * v1 / range1;
      break;
    case 4: /* concave, unipolar, positive */
      v1 = fluid_concave(v1);
      break;
    case 5: /* concave, unipolar, negative */
      v1 = fluid_concave(127 - v1);
      break;
    case 6: /* concave, bipolar, positive */
      v1 = (v1 > 64)? fluid_concave(2 * (v1 - 64)) : -fluid_concave(2 * (64 - v1));
      break;
    case 7: /* concave, bipolar, negative */
      v1 = (v1 > 64)? -fluid_concave(2 * (v1 - 64)) : fluid_concave(2 * (64 - v1));
      break;
    case 8: /* convex, unipolar, positive */
      v1 = fluid_convex(v1);
      break;
    case 9: /* convex, unipolar, negative */
      v1 = fluid_convex(127 - v1);
      break;
    case 10: /* convex, bipolar, positive */
      v1 = (v1 > 64)? fluid_convex(2 * (v1 - 64)) : -fluid_convex(2 * (64 - v1));
      break;
    case 11: /* convex, bipolar, negative */
      v1 = (v1 > 64)? -fluid_convex(2 * (v1 - 64)) : fluid_convex(2 * (64 - v1));
      break;
    case 12: /* switch, unipolar, positive */
      v1 = (v1 >= 64)? 1.0f : 0.0f;
      break;
    case 13: /* switch, unipolar, negative */
      v1 = (v1 >= 64)? 0.0f : 1.0f;
      break;
    case 14: /* switch, bipolar, positive */
      v1 = (v1 >= 64)? 1.0f : -1.0f;
      break;
    case 15: /* switch, bipolar, negative */
      v1 = (v1 >= 64)? -1.0f : 1.0f;
      break;
    }
  } else {
    return 0.0;
  }

  /* no need to go further */
  if (v1 == 0.0f) {
    return 0.0f;
  }

  /* get the second input source */
  if (mod->src2 > 0) {
    if (mod->flags2 & FLUID_MOD_CC) {
      v2 = fluid_channel_get_cc(chan, mod->src2);
    } else {
      switch (mod->src2) {
      case FLUID_MOD_NONE:         /* SF 2.01 8.2.1 item 0: src enum=0 => value is 1 */
	v2 = range2;
	break;
      case FLUID_MOD_VELOCITY:
	v2 = voice->vel;
	break;
      case FLUID_MOD_KEY:
	v2 = voice->key;
	break;
      case FLUID_MOD_KEYPRESSURE:
	v2 = chan->key_pressure;
	break;
      case FLUID_MOD_CHANNELPRESSURE:
	v2 = chan->channel_pressure;
	break;
      case FLUID_MOD_PITCHWHEEL:
	v2 = chan->pitch_bend;
	break;
      case FLUID_MOD_PITCHWHEELSENS:
	v2 = chan->pitch_wheel_sensitivity;
	break;
      default:
	v1 = 0.0f;
      }
    }

    /* transform the second input value */
    switch (mod->flags2 & 0x0f) {
    case 0: /* linear, unipolar, positive */
      v2 /= range2;
      break;
    case 1: /* linear, unipolar, negative */
      v2 = 1.0f - v2 / range2;
      break;
    case 2: /* linear, bipolar, positive */
      v2 = -1.0f + 2.0f * v2 / range2;
      break;
    case 3: /* linear, bipolar, negative */
      v2 = -1.0f + 2.0f * v2 / range2;
      break;
    case 4: /* concave, unipolar, positive */
      v2 = fluid_concave(v2);
      break;
    case 5: /* concave, unipolar, negative */
      v2 = fluid_concave(127 - v2);
      break;
    case 6: /* concave, bipolar, positive */
      v2 = (v2 > 64)? fluid_concave(2 * (v2 - 64)) : -fluid_concave(2 * (64 - v2));
      break;
    case 7: /* concave, bipolar, negative */
      v2 = (v2 > 64)? -fluid_concave(2 * (v2 - 64)) : fluid_concave(2 * (64 - v2));
      break;
    case 8: /* convex, unipolar, positive */
      v2 = fluid_convex(v2);
      break;
    case 9: /* convex, unipolar, negative */
      v2 = 1.0f - fluid_convex(v2);
      break;
    case 10: /* convex, bipolar, positive */
      v2 = (v2 > 64)? -fluid_convex(2 * (v2 - 64)) : fluid_convex(2 * (64 - v2));
      break;
    case 11: /* convex, bipolar, negative */
      v2 = (v2 > 64)? -fluid_convex(2 * (v2 - 64)) : fluid_convex(2 * (64 - v2));
      break;
    case 12: /* switch, unipolar, positive */
      v2 = (v2 >= 64)? 1.0f : 0.0f;
      break;
    case 13: /* switch, unipolar, negative */
      v2 = (v2 >= 64)? 0.0f : 1.0f;
      break;
    case 14: /* switch, bipolar, positive */
      v2 = (v2 >= 64)? 1.0f : -1.0f;
      break;
    case 15: /* switch, bipolar, negative */
      v2 = (v2 >= 64)? -1.0f : 1.0f;
      break;
    }
  } else {
    v2 = 1.0f;
  }

  /* it's as simple as that: */
  return (fluid_real_t) mod->amount * v1 * v2;
}

/*
 * fluid_mod_new
 */
fluid_mod_t*
fluid_mod_new()
{
  fluid_mod_t* mod = FLUID_NEW(fluid_mod_t);
  if (mod == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  return mod;
};

/*
 * fluid_mod_delete
 */
void
fluid_mod_delete(fluid_mod_t * mod)
{
  FLUID_FREE(mod);
};

/*
 * fluid_mod_test_identity
 */
/* Purpose:
 * Checks, if two modulators are identical.
 *  SF2.01 section 9.5.1 page 69, 'bullet' 3 defines 'identical'.
 */
int fluid_mod_test_identity(fluid_mod_t * mod1, fluid_mod_t * mod2){
  if (mod1->dest != mod2->dest){return 0;};
  if (mod1->src1 != mod2->src1){return 0;};
  if (mod1->src2 != mod2->src2){return 0;};
  if (mod1->flags1 != mod2->flags1){return 0;}
  if (mod1->flags2 != mod2->flags2){return 0;}
  return 1;
};

/* debug function: Prints the contents of a modulator */
void fluid_dump_modulator(fluid_mod_t * mod){
  int src1=mod->src1;
  int dest=mod->dest;
  int src2=mod->src2;
  int flags1=mod->flags1;
  int flags2=mod->flags2;
  fluid_real_t amount=(fluid_real_t)mod->amount;

  printf("Src: ");
  if (flags1 & FLUID_MOD_CC){
    printf("MIDI CC=%i",src1);
  } else {
    switch(src1){
	case FLUID_MOD_NONE:
	  printf("None"); break;
	case FLUID_MOD_VELOCITY:
	  printf("note-on velocity"); break;
	case FLUID_MOD_KEY:
	  printf("Key nr"); break;
	  case FLUID_MOD_KEYPRESSURE:
	    printf("Poly pressure"); break;
	case FLUID_MOD_CHANNELPRESSURE:
	  printf("Chan pressure"); break;
	case FLUID_MOD_PITCHWHEEL:
	  printf("Pitch Wheel"); break;
	case FLUID_MOD_PITCHWHEELSENS:
	  printf("Pitch Wheel sens"); break;
	default:
	  printf("(unknown: %i)", src1);
    }; /* switch src1 */
  }; /* if not CC */
  if (flags1 & FLUID_MOD_NEGATIVE){printf("- ");} else {printf("+ ");};
  if (flags1 & FLUID_MOD_BIPOLAR){printf("bip ");} else {printf("unip ");};
  printf("-> ");
  switch(dest){
      case GEN_FILTERQ: printf("Q"); break;
      case GEN_FILTERFC: printf("fc"); break;
      case GEN_VIBLFOTOPITCH: printf("VibLFO-to-pitch"); break;
      case GEN_MODENVTOPITCH: printf("ModEnv-to-pitch"); break;
      case GEN_MODLFOTOPITCH: printf("ModLFO-to-pitch"); break;
      case GEN_CHORUSSEND: printf("Chorus send"); break;
      case GEN_REVERBSEND: printf("Reverb send"); break;
      case GEN_PAN: printf("pan"); break;
      case GEN_ATTENUATION: printf("att"); break;
      default: printf("dest %i",dest);
  }; /* switch dest */
  printf(", amount %f flags %i src2 %i flags2 %i\n",amount, flags1, src2, flags2);
};


