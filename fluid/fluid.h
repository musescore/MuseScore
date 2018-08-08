/*
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


#ifndef __FLUID_S_H__
#define __FLUID_S_H__

#include "synthesizer/synthesizer.h"
#include "synthesizer/midipatch.h"

namespace FluidS {

using namespace Ms;

class Voice;
class SFont;
class Preset;
class Sample;
class Channel;
struct Mod;
class Fluid;

#define FLUID_NUM_PROGRAMS      129

enum fluid_loop {
      FLUID_UNLOOPED            = 0,
      FLUID_LOOP_DURING_RELEASE = 1,
      FLUID_NOTUSED             = 2,
      FLUID_LOOP_UNTIL_RELEASE  = 3
      };

enum fluid_synth_status {
      FLUID_SYNTH_CLEAN,
      FLUID_SYNTH_PLAYING,
      FLUID_SYNTH_QUIET,
      FLUID_SYNTH_STOPPED
      };

//---------------------------------------------------------
//   BankOffset
//---------------------------------------------------------

struct BankOffset {
      int sfont_id;
      int offset;
      };

enum fluid_midi_control_change {
      BANK_SELECT_MSB = 0x00,
      MODULATION_MSB = 0x01,
      BREATH_MSB = 0x02,
      FOOT_MSB = 0x04,
      PORTAMENTO_TIME_MSB = 0x05,
      DATA_ENTRY_MSB = 0x06,
      VOLUME_MSB = 0x07,
      BALANCE_MSB = 0x08,
      PAN_MSB = 0x0A,
      EXPRESSION_MSB = 0x0B,
      EFFECTS1_MSB = 0x0C,
      EFFECTS2_MSB = 0x0D,
      GPC1_MSB = 0x10, /* general purpose controller */
      GPC2_MSB = 0x11,
      GPC3_MSB = 0x12,
      GPC4_MSB = 0x13,
      BANK_SELECT_LSB = 0x20,
      MODULATION_WHEEL_LSB = 0x21,
      BREATH_LSB = 0x22,
      FOOT_LSB = 0x24,
      PORTAMENTO_TIME_LSB = 0x25,
      DATA_ENTRY_LSB = 0x26,
      VOLUME_LSB = 0x27,
      BALANCE_LSB = 0x28,
      PAN_LSB = 0x2A,
      EXPRESSION_LSB = 0x2B,
      EFFECTS1_LSB = 0x2C,
      EFFECTS2_LSB = 0x2D,
      GPC1_LSB = 0x30,
      GPC2_LSB = 0x31,
      GPC3_LSB = 0x32,
      GPC4_LSB = 0x33,
      SUSTAIN_SWITCH = 0x40,
      PORTAMENTO_SWITCH = 0x41,
      SOSTENUTO_SWITCH = 0x42,
      SOFT_PEDAL_SWITCH = 0x43,
      LEGATO_SWITCH = 0x45,
      HOLD2_SWITCH = 0x45,
      SOUND_CTRL1 = 0x46,
      SOUND_CTRL2 = 0x47,
      SOUND_CTRL3 = 0x48,
      SOUND_CTRL4 = 0x49,
      SOUND_CTRL5 = 0x4A,
      SOUND_CTRL6 = 0x4B,
      SOUND_CTRL7 = 0x4C,
      SOUND_CTRL8 = 0x4D,
      SOUND_CTRL9 = 0x4E,
      SOUND_CTRL10 = 0x4F,
      GPC5 = 0x50,
      GPC6 = 0x51,
      GPC7 = 0x52,
      GPC8 = 0x53,
      PORTAMENTO_CTRL = 0x54,
      EFFECTS_DEPTH1 = 0x5B,
      EFFECTS_DEPTH2 = 0x5C,
      EFFECTS_DEPTH3 = 0x5D,
      EFFECTS_DEPTH4 = 0x5E,
      EFFECTS_DEPTH5 = 0x5F,
      DATA_ENTRY_INCR = 0x60,
      DATA_ENTRY_DECR = 0x61,
      NRPN_LSB = 0x62,
      NRPN_MSB = 0x63,
      RPN_LSB = 0x64,
      RPN_MSB = 0x65,
      ALL_SOUND_OFF = 0x78,
      ALL_CTRL_OFF = 0x79,
      LOCAL_CONTROL = 0x7A,
      ALL_NOTES_OFF = 0x7B,
      OMNI_OFF = 0x7C,
      OMNI_ON = 0x7D,
      POLY_OFF = 0x7E,
      POLY_ON = 0x7F
      };

/**
 * Generator (effect) numbers (SoundFont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
  GEN_STARTADDROFS,		/**< Sample start address offset (0-32767) */
  GEN_ENDADDROFS,		      /**< Sample end address offset (-32767-0) */
  GEN_STARTLOOPADDROFS,		/**< Sample loop start address offset (-32767-32767) */
  GEN_ENDLOOPADDROFS,		/**< Sample loop end address offset (-32767-32767) */
  GEN_STARTADDRCOARSEOFS,	/**< Sample start address coarse offset (X 32768) */
  GEN_MODLFOTOPITCH,		/**< Modulation LFO to pitch */
  GEN_VIBLFOTOPITCH,		/**< Vibrato LFO to pitch */
  GEN_MODENVTOPITCH,		/**< Modulation envelope to pitch */
  GEN_FILTERFC,			/**< Filter cutoff */
  GEN_FILTERQ,			/**< Filter Q */
  GEN_MODLFOTOFILTERFC,		/**< Modulation LFO to filter cutoff */
  GEN_MODENVTOFILTERFC,		/**< Modulation envelope to filter cutoff */
  GEN_ENDADDRCOARSEOFS,		/**< Sample end address coarse offset (X 32768) */
  GEN_MODLFOTOVOL,		/**< Modulation LFO to volume */
  GEN_UNUSED1,			/**< Unused */
  GEN_CHORUSSEND,		      /**< Chorus send amount */
  GEN_REVERBSEND,		      /**< Reverb send amount */
  GEN_PAN,			      /**< Stereo panning */
  GEN_UNUSED2,			/**< Unused */
  GEN_UNUSED3,			/**< Unused */
  GEN_UNUSED4,			/**< Unused */
  GEN_MODLFODELAY,		/**< Modulation LFO delay */
  GEN_MODLFOFREQ,		      /**< Modulation LFO frequency */
  GEN_VIBLFODELAY,		/**< Vibrato LFO delay */
  GEN_VIBLFOFREQ,		      /**< Vibrato LFO frequency */
  GEN_MODENVDELAY,		/**< Modulation envelope delay */
  GEN_MODENVATTACK,		/**< Modulation envelope attack */
  GEN_MODENVHOLD,		      /**< Modulation envelope hold */
  GEN_MODENVDECAY,		/**< Modulation envelope decay */
  GEN_MODENVSUSTAIN,		/**< Modulation envelope sustain */
  GEN_MODENVRELEASE,		/**< Modulation envelope release */
  GEN_KEYTOMODENVHOLD,		/**< Key to modulation envelope hold */
  GEN_KEYTOMODENVDECAY,		/**< Key to modulation envelope decay */
  GEN_VOLENVDELAY,		/**< Volume envelope delay */
  GEN_VOLENVATTACK,		/**< Volume envelope attack */
  GEN_VOLENVHOLD,		      /**< Volume envelope hold */
  GEN_VOLENVDECAY,		/**< Volume envelope decay */
  GEN_VOLENVSUSTAIN,		/**< Volume envelope sustain */
  GEN_VOLENVRELEASE,		/**< Volume envelope release */
  GEN_KEYTOVOLENVHOLD,		/**< Key to volume envelope hold */
  GEN_KEYTOVOLENVDECAY,		/**< Key to volume envelope decay */
  GEN_INSTRUMENT,		      /**< Instrument ID (shouldn't be set by user) */
  GEN_RESERVED1,		      /**< Reserved */
  GEN_KEYRANGE,			/**< MIDI note range */
  GEN_VELRANGE,			/**< MIDI velocity range */
  GEN_STARTLOOPADDRCOARSEOFS,	/**< Sample start loop address coarse offset (X 32768) */
  GEN_KEYNUM,			/**< Fixed MIDI note number */
  GEN_VELOCITY,			/**< Fixed MIDI velocity value */
  GEN_ATTENUATION,		/**< Initial volume attenuation */
  GEN_RESERVED2,		      /**< Reserved */
  GEN_ENDLOOPADDRCOARSEOFS,	/**< Sample end loop address coarse offset (X 32768) */
  GEN_COARSETUNE,		      /**< Coarse tuning */
  GEN_FINETUNE,			/**< Fine tuning */
  GEN_SAMPLEID,			/**< Sample ID (shouldn't be set by user) */
  GEN_SAMPLEMODE,		      /**< Sample mode flags */
  GEN_RESERVED3,		      /**< Reserved */
  GEN_SCALETUNE,		      /**< Scale tuning */
  GEN_EXCLUSIVECLASS,		/**< Exclusive class number */
  GEN_OVERRIDEROOTKEY,		/**< Sample root note override */

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,			/**< Pitch (NOTE: Not a real SoundFont generator) */
  GEN_LAST			      /**< Value defines the count of generators (#fluid_gen_type) */
      };

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel {
      Fluid* synth;

      unsigned int sfontnum;
      unsigned int banknum;
      unsigned int prognum;
      Preset* _preset;

   public:
      int channum;
      short key_pressure;
      short channel_pressure;
      short pitch_bend;
      short pitch_wheel_sensitivity;

      short cc[128];          // controller values

      /* cached values of last MSB values of MSB/LSB controllers */
      unsigned char bank_msb;
      int interp_method;

      /* NRPN system */
      short nrpn_select;

      /* The values of the generators, set by NRPN messages, or by
       * fluid_synth_set_gen(), are cached in the channel so they can be
       * applied to future notes. They are copied to a voice's generators
       * in fluid_voice_init(), wihich calls fluid_gen_init().  */

      float gen[GEN_LAST];

      /* By default, the NRPN values are relative to the values of the
       * generators set in the SoundFont. For example, if the NRPN
       * specifies an attack of 100 msec then 100 msec will be added to the
       * combined attack time of the sound font and the modulators.
       *
       * However, it is useful to be able to specify the generator value
       * absolutely, completely ignoring the generators of the sound font
       * and the values of modulators. The gen_abs field, is a boolean
       * flag indicating whether the NRPN value is absolute or not.
       */
      char gen_abs[GEN_LAST];

   public:
      Channel(Fluid* synth, int num);

      bool sustained() const              { return cc[SUSTAIN_SWITCH] >= 64; }
      void setGen(int n, float v, char a) { gen[n] = v; gen_abs[n] = a; }
      float getGen(int n) const           { return gen[n]; }
      char getGenAbs(int n) const         { return gen_abs[n]; }
      void init();
      void initCtrl();
      void setCC(int n, int val)          { cc[n] = val; }
      void reset();
      void setPreset(Preset* p);
      Preset* preset() const              { return _preset;  }
      unsigned int getSfontnum() const    { return sfontnum; }
      void setSfontnum(unsigned int s)    { sfontnum = s;    }
      unsigned int getBanknum() const     { return banknum;  }
      void setBanknum(unsigned int b)     { banknum = b;     }
      void setPrognum(int p)              { prognum = p;     }
      int getPrognum() const              { return prognum;  }
      void setcc(int ctrl, int val);
      void pitchBend(int val);
      int getPitchBend() const            { return pitch_bend; }
      void pitchWheelSens(int val);
      int getCC(int num);
      int getNum() const                  { return channum;    }
      void setInterpMethod(int m)         { interp_method = m; }
      int getInterpMethod() const         { return interp_method; }
      };

// subsystems:
enum {
      FLUID_GROUP  = 0,
      };

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

class Fluid : public Synthesizer {
      QList<SFont*> sfonts;               // the loaded soundfonts
      QList<MidiPatch*> patches;

      QList<Voice*> freeVoices;           // unused synthesis processes
      QList<Voice*> activeVoices;         // active synthesis processes
      QString _error;                     // last error message

      static bool initialized;

      double sample_rate;                 // The sample rate
      float _masterTuning;                // usually 440.0
      double _tuning[128];                // the pitch of every key, in cents

      int _loadProgress = 0;
      bool _loadWasCanceled = false;

      QMutex mutex;
      void updatePatchList();

   protected:
      int _state;                         // the synthesizer state

      unsigned int sfont_id;

      QList<Channel*> channel;            // the channels

      unsigned int noteid;                // the id is incremented for every new note. it's used for noteoff's

      SFont* get_sfont_by_name(const QString& name);
      SFont* get_sfont_by_id(int id);
      SFont* get_sfont(int idx) const     { return sfonts[idx];   }
      bool sfunload(int id);
      int sfload(const QString& filename);

   public:
      Fluid();
      ~Fluid();
      virtual void init(float sampleRate);

      virtual const char* name() const { return "Fluid"; }

      virtual void play(const PlayEvent&);
      virtual const QList<MidiPatch*>& getPatchInfo() const { return patches; }

      // get/set synthesizer state (parameter set)
      virtual SynthesizerGroup state() const;
      virtual bool setState(const SynthesizerGroup&);

      virtual void allSoundsOff(int);
      virtual void allNotesOff(int);

      int loadProgress()            { return _loadProgress; }
      void setLoadProgress(int val) { _loadProgress = val; }
      bool loadWasCanceled()        { return _loadWasCanceled; }
      void setLoadWasCanceled(bool status)     { _loadWasCanceled = status; }

      Preset* get_preset(unsigned int sfontnum, unsigned int banknum, unsigned int prognum);
      Preset* find_preset(unsigned int banknum, unsigned int prognum);
      void modulate_voices(int chan, bool is_cc, int ctrl);
      void modulate_voices_all(int chan);
      void damp_voices(int chan);
      int kill_voice(Voice * voice);
      void print_voice();

      /** This function assures that every MIDI channels has a valid preset
       *  (NULL is okay). This function is called after a SoundFont is
       *  unloaded or reloaded. */
      void update_presets();

      int get_cc(int chan, int num) const { return channel[chan]->cc[num]; }

      void system_reset();
      void program_change(int chan, int prognum);

      void set_gen2(int chan, int param, float value, int absolute, int normalized);
      float get_gen(int chan, int param);
      void set_gen(int chan, int param, float value);
      void set_interp_method(int chan, int interp_method);

      Preset* get_channel_preset(int chan) const { return channel[chan]->preset(); }

      virtual bool loadSoundFonts(const QStringList& s);
      virtual bool addSoundFont(const QString& s);
      virtual bool removeSoundFont(const QString& s);
      virtual QStringList soundFonts() const;

      void start_voice(Voice* voice);
      Voice* alloc_voice(unsigned id, Sample* sample, int chan, int key, int vel, double vt);
      void free_voice_by_kill();

      virtual void process(unsigned len, float* out, float* effect1, float* effect2);

      bool program_select(int chan, unsigned sfont_id, unsigned bank_num, unsigned preset_num);
      void get_program(int chan, unsigned* sfont_id, unsigned* bank_num, unsigned* preset_num);
//      void sfont_select(int chan, unsigned int sfont_id)    { channel[chan]->setSfontnum(sfont_id); }
//      void bank_select(int chan, unsigned int bank)         { channel[chan]->setBanknum(bank); }

      void get_pitch_wheel_sens(int chan, int* pval);
      void pitch_wheel_sens(int chan, int val);
      void get_pitch_bend(int chan, int* ppitch_bend);

      void freeVoice(Voice* v);

      double getPitch(int k) const   { return _tuning[k]; }
      float ct2hz_real(float cents)  { return powf(2.0f, (cents - 6900.0f) / 1200.0f) * _masterTuning; }

      float act2hz(float c)          { return 8.176 * pow(2.0, (double) c / 1200.0); }
      float ct2hz(float cents)       { return act2hz(qBound(1500.0f, cents, 13500.0f)); }

      virtual double masterTuning() const     { return _masterTuning; }
      virtual void setMasterTuning(double f)  { _masterTuning = f;    }

      QString error() const { return _error; }

      virtual SynthesizerGui* gui();

      static QFileInfoList sfFiles();

      friend class Voice;
      friend class Preset;
      };

  /*
   *
   * Chorus
   *
   */

enum fluid_chorus_mod {
      FLUID_CHORUS_MOD_SINE = 0,
      FLUID_CHORUS_MOD_TRIANGLE = 1
      };

/* Those are the default settings for the chorus. */
#define FLUID_CHORUS_DEFAULT_N      3
#define FLUID_CHORUS_DEFAULT_LEVEL  2.0f
#define FLUID_CHORUS_DEFAULT_SPEED  0.3f
#define FLUID_CHORUS_DEFAULT_DEPTH  8.0f
#define FLUID_CHORUS_DEFAULT_TYPE   FLUID_CHORUS_MOD_SINE


  /*
   *
   * Synthesis parameters
   *
   */

  /* Flags to choose the interpolation method */
enum fluid_interp {
      /* no interpolation: Fastest, but questionable audio quality */
      FLUID_INTERP_NONE     = 0,
      /* Straight-line interpolation: A bit slower, reasonable audio quality */
      FLUID_INTERP_LINEAR   = 1,
      /* Fourth-order interpolation: Requires 50 % of the whole DSP processing time, good quality
       * Default. */
      FLUID_INTERP_DEFAULT  = 4,
      FLUID_INTERP_4THORDER = 4,
      FLUID_INTERP_7THORDER = 7,
      FLUID_INTERP_HIGHEST  = 7
      };

#define fluid_sample_refcount(_sample) ((_sample)->refcount)


/** Sample types */

enum {
      FLUID_SAMPLETYPE_MONO =	      1,
      FLUID_SAMPLETYPE_RIGHT =	2,
      FLUID_SAMPLETYPE_LEFT =	      4,
      FLUID_SAMPLETYPE_LINKED =	8,
      FLUID_SAMPLETYPE_OGG_VORBIS = 0x10,
      FLUID_SAMPLETYPE_ROM =	      0x8000
      };

/* Sets the sound data of the sample
 *     Warning : if copy_data is FALSE, data should have 8 unused frames at start
 *     and 8 unused frames at the end.
 */
int fluid_sample_set_sound_data(Sample* sample, short *data,
			       unsigned int nbframes, short copy_data, int rootkey);

/*
 *
 *  Utility functions
 */

  /* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD           64

/**
 * SoundFont generator structure.
 */
class Generator {
   public:
      unsigned char flags; /**< Is the generator set or not (#fluid_gen_flags) */
      double val;          /**< The nominal value           */
      double mod;          /**< Change by modulators        */
      double nrpn;         /**< Change by NRPN messages     */

      void set_mod(double _val)  { mod  = _val; }
      void set_nrpn(double _val) { nrpn = _val; }
      };

/**
 * Enum value for 'flags' field of #_Generator (not really flags).
 */
enum fluid_gen_flags {
      GEN_UNUSED,		/**< Generator value is not set */
      GEN_SET,		/**< Generator value is set     */
      GEN_ABS_NRPN	/**< DOCME                      */
      };

void fluid_gen_set_default_values(Generator* gen);
  /*
   *  The interface to the synthesizer's voices
   *  Examples on using them can be found in fluid_defsfont.c
   */

  /* for fluid_voice_add_mod */
enum fluid_voice_add_mod {
      FLUID_VOICE_OVERWRITE,
      FLUID_VOICE_ADD,
      FLUID_VOICE_DEFAULT
      };

/* Disable FPE exception check */
#define fluid_check_fpe(expl)

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);

/*
 * interpolation data
 */
struct fluid_interp_coeff_t {
      float a0, a1, a2, a3;
      };

/* Flags telling the polarity of a modulator.  Compare with SF2.01
   section 8.2. Note: The numbers of the bits are different!  (for
   example: in the flags of a SF modulator, the polarity bit is bit
   nr. 9) */

enum fluid_mod_flags {
      FLUID_MOD_POSITIVE = 0,
      FLUID_MOD_NEGATIVE = 1,
      FLUID_MOD_UNIPOLAR = 0,
      FLUID_MOD_BIPOLAR  = 2,
      FLUID_MOD_LINEAR   = 0,
      FLUID_MOD_CONCAVE  = 4,
      FLUID_MOD_CONVEX   = 8,
      FLUID_MOD_SWITCH   = 12,
      FLUID_MOD_GC       = 0,
      FLUID_MOD_CC       = 16
      };

//---------------------------------------------------------
//   Mod
//---------------------------------------------------------

struct Mod
      {
      unsigned char dest;
      unsigned char src1;
      unsigned char flags1;
      unsigned char src2;
      unsigned char flags2;
      double amount;

      void clone(Mod* mod) const;
      void dump() const;
      int has_source(bool cc, int ctrl) {
            return (((src1 == ctrl) && (flags1 & FLUID_MOD_CC)    && cc)
                || (((src1 == ctrl) && (!(flags1 & FLUID_MOD_CC)) && !cc)))
                || (((src2 == ctrl) && (flags2 & FLUID_MOD_CC)    && cc)
                || (((src2 == ctrl) && (!(flags2 & FLUID_MOD_CC)) && !cc)));
            }
      void set_source1(int src, int flags);
      void set_source2(int src, int flags);
      void set_dest(int val)                    { dest = val;    }
      void set_amount(double val)               { amount = val;  }
      int get_source1() const                   { return src1;   }
      int get_flags1() const                    { return flags1; }
      int get_source2() const                   { return src2;   }
      int get_flags2() const                    { return flags2; }
      int get_dest() const                      { return dest;   }
      double get_amount() const                 { return amount; }
      float get_value(Channel* chan, Voice* voice);
      };

/* Flags telling the source of a modulator.  This corresponds to
 * SF2.01 section 8.2.1 */

enum fluid_mod_src {
      FLUID_MOD_NONE             = 0,
      FLUID_MOD_VELOCITY         = 2,
      FLUID_MOD_KEY              = 3,
      FLUID_MOD_KEYPRESSURE      = 10,
      FLUID_MOD_CHANNELPRESSURE  = 13,
      FLUID_MOD_PITCHWHEEL       = 14,
      FLUID_MOD_PITCHWHEELSENS   = 16
      };

/* Determines, if two modulators are 'identical' (all parameters
   except the amount match) */
bool test_identity(const Mod * mod1, const Mod * mod2);

void fluid_dump_modulator(Mod * mod);

#define fluid_mod_has_source(mod,cc,ctrl)  \
( ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) != 0) && (cc != 0)) \
   || ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) == 0) && (cc == 0)))) \
|| ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) != 0) && (cc != 0)) \
    || ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) == 0) && (cc == 0)))))

#define fluid_mod_has_dest(mod,gen)  ((mod)->dest == gen)

/*
 *  phase
 */

#define FLUID_INTERP_BITS        8
#define FLUID_INTERP_BITS_MASK   0xff000000
#define FLUID_INTERP_BITS_SHIFT  24
#define FLUID_INTERP_MAX         256

#define FLUID_FRACT_MAX ((double)4294967296.0)

//---------------------------------------------------------
//   Phase
/* Purpose:
* Playing pointer for voice playback
*
* When a sample is played back at a different pitch, the playing pointer in the
* source sample will not advance exactly one sample per output sample.
* This playing pointer is implemented using Phase.
* It is a 64 bit number. The higher 32 bits contain the 'index' (number of
* the current sample), the lower 32 bits the fractional part.
* Access is possible in two ways:
* -through the 64 bit part 'b64', if the architecture supports 64 bit integers
* -through 'index' and 'fract'
* Note: b64 and index / fract share the same memory location!
*/

struct Phase {
      qint64 data;

      void operator+=(const Phase& p) { data += p.data; }
      void setInt(qint32 b)           { data = qint64(b) << 32; }
      void setFloat(double b)          {
             data = (((qint64)(b)) << 32) | (quint32) (((double)(b) - (int)(b)) * (double)FLUID_FRACT_MAX);
            }

      void operator-=(const Phase& b) { data -= b.data;  }
      void operator-=(int b)          { data -= (qint64(b) << 32);  }
      int index() const               { return data >> 32; }
      quint32 fract() const           { return quint32(data & 0xffffffff); }
      quint32 index_round() const     { return quint32((data+0x80000000) >> 32); }

      Phase() {}
      Phase(qint64 v) : data(v) {}
      };

/* Purpose:
 * Takes the fractional part of the argument phase and
 * calculates the corresponding position in the interpolation table.
 * The fractional position of the playing pointer is calculated with a quite high
 * resolution (32 bits). It would be unpractical to keep a set of interpolation
 * coefficients for each possible fractional part...
 */
#define fluid_phase_fract_to_tablerow(_x) \
  ((int)(((_x).fract() & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT))

#define fluid_phase_double(_x) \
  ((double)((_x).index()) + ((double)((_x).fract()) / FLUID_FRACT_MAX))

/* Purpose:
 * The playing pointer is _phase. How many output samples are produced, until the point _p1 in the sample is reached,
 * if _phase advances in steps of _incr?
 */
#define fluid_phase_steps(_phase,_idx,_incr) \
  (int)(((double)(_idx) - fluid_phase_double(_phase)) / (double)_incr)

/* Purpose:
 * Creates the expression a.index++.
*/
#define fluid_phase_index_plusplus(a) (((a)._index)++)

}  // namespace Fluid

#endif  // __FLUID_S_H__
