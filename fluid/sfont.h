/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont loading code borrowed from Smurf SoundFont Editor by Josh Green
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

#ifndef _FLUID_DEFSFONT_H
#define _FLUID_DEFSFONT_H

#include "config.h"
#include "fluid.h"

namespace FluidS {

class Preset;
class Sample;
class Instrument;
struct SFGen;
struct SFMod;

struct SFChunk;

//---------------------------------------------------------
//   SFVersion
//---------------------------------------------------------

struct SFVersion {            // version structure
      unsigned short major, minor;
      SFVersion();
      };

//---------------------------------------------------------
//   SFont
//---------------------------------------------------------

class SFont {
      Fluid* synth;
      QFile f;
      unsigned samplepos;           // the position in the file at which the sample data starts
      unsigned samplesize;          // the size of the sample data

      QList<Instrument*> instruments;
      QList<Preset*> presets;
      QList<Sample*> sample;

      int _id;
      int _bankOffset;

      SFVersion _version;		// sound font version
      SFVersion romver;		      // ROM version
      QString _fontName;
      QList<unsigned char*> infos;	// list of info strings (1st byte is ID)

      void read_listchunk(SFChunk* chunk);
      void process_info(int size);
      void process_sdta(unsigned int size);
      void pdtahelper(unsigned int expid, unsigned int reclen, SFChunk* chunk, int* size);

      void process_pdta(int size);
      void load_phdr(int size);
      void load_pbag(int size);
      void load_pmod(int size);
      void load_pgen(int size);
      void load_ihdr(int size);
      void load_ibag(int size);
      void load_imod(int size);
      void load_igen(int size);
      void load_shdr(int size);

      void fixup_pgen();
      void fixup_igen();
      void fixup_sample();

      void readchunk(SFChunk*);
      unsigned short READW();
      void READD(unsigned int& var);
      void FSKIP(int size)    {  return safe_fseek(size); }
      void FSKIPW();
      unsigned char READB();
      char READC();
      void READSTR(char*);

      void safe_fread(void *buf, int count);
      void safe_fseek(long ofs);
      bool load();

   public:
      SFont(Fluid* f);
      virtual ~SFont();

      QString get_name()  const                 { return f.fileName(); }
      Preset* get_preset(int bank, int prenum);

      bool read(const QString& file);

      int load_sampledata();
      unsigned int samplePos() const            { return samplepos;  }
      int id() const                            { return _id; }
      void setId(int i)                         { _id = i;    }
      void setSamplepos(unsigned v)             { samplepos = v; }
      void setSamplesize(unsigned v)            { samplesize = v; }
      unsigned getSamplesize() const            { return samplesize; }
      const QList<Preset*> getPresets() const   { return presets; }
      SFVersion version() const                 { return _version; }
      int bankOffset() const                    { return _bankOffset; }
      void setBankOffset(int val)               { _bankOffset = val; }
      QString fontName() const                  { return _fontName; }

      friend class Preset;
      };

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample {
      bool _valid;

   public:
      SFont* sf;
      unsigned int start;
      unsigned int end;
      unsigned int loopstart;
      unsigned int loopend;
      unsigned int samplerate;
      int origpitch;
      int pitchadj;
      int sampletype;

      short* data;

      /** The amplitude, that will lower the level of the sample's loop to
          the noise floor. Needed for note turnoff optimization, will be
          filled out automatically */
      /* Set this to zero, when submitting a new sample. */

      bool amplitude_that_reaches_noise_floor_is_valid;
      double amplitude_that_reaches_noise_floor;

      Sample(SFont*);
      ~Sample();

      bool inRom() const;
      void optimize();
      void load();
      bool valid() const    { return _valid; }
      void setValid(bool v) { _valid = v; }
#ifdef SOUNDFONT3
      bool decompressOggVorbis(char* p, int size);
#endif
      };

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

class Zone {
   public:
      union {
            Sample* sample;
            int sampIdx;
            Instrument* instrument;
            int instIdx;
            };

      QList<SFGen*> gen;
      QList<SFMod*> mod;

      int keylo, keyhi, vello, velhi;
      Generator   genlist[GEN_LAST];
      QList<Mod*> modlist;        // List of modulators

   public:
      Zone();
      ~Zone();
      bool importZone();
      bool inside_range(int key, int vel) const;
      Instrument* get_inst()     const          { return instrument; }
      Sample* get_sample() const                { return sample; }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class Instrument {
   public:
      Zone* global_zone;
      QList<Zone*> zones;

   public:
      Instrument();
      ~Instrument();
      Zone* get_global_zone() const     { return global_zone; }
      QList<Zone*> get_zone()           { return zones; }
      bool import_sfont();
      };

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

class Preset {
   public:
      QString name;                 // the name of the preset
      SFont* sfont;
      int bank;                     // the bank number
      int num;                      // the preset number

      Zone* _global_zone;           // the global zone of the preset
      QList<Zone*> zones;

   public:
      Preset(SFont* sfont);
      ~Preset();

      QString get_name() const                  { return name; }
      int get_banknum() const                   { return bank; }
      int get_num() const                       { return num;  }
      bool noteon(Fluid*, unsigned id, int chan, int key, int vel, double nt);

      void setGlobalZone(Zone* z)               { _global_zone = z;   }
      bool importSfont();

      Zone* global_zone()                       { return _global_zone; }
      void loadSamples();
      QList<Zone*> getZones()                   { return zones; }
      };

//---------------------------------------------------------
//   SFChunk
//---------------------------------------------------------

struct SFChunk {              // RIFF file chunk structure
      unsigned int id;	      // chunk id
      unsigned int size;	// size of the following chunk
      };

struct SFMod {				/* Modulator structure */
      unsigned short src;		/* source modulator */
      unsigned short dest;		/* destination generator */
      signed short amount;		/* signed, degree of modulation */
      unsigned short amtsrc;		/* second source controls amnt of first */
      unsigned short trans;		/* transform applied to source */
      };

union SFGenAmount {			/* Generator amount structure */
      signed short sword;		/* signed 16 bit value */
      unsigned short uword;		/* unsigned 16 bit value */
      struct {
            unsigned char lo;		/* low value for ranges */
            unsigned char hi;		/* high value for ranges */
            } range;
      };

struct SFGen {				/* Generator structure  */
      unsigned short id;		/* generator ID         */
      SFGenAmount amount;		/* generator value      */
      };


/* NOTE: sffd is also used to determine if sound font is new (NULL) */

/* sf file chunk IDs */
enum {
      UNKN_ID, RIFF_ID, LIST_ID, SFBK_ID,
      INFO_ID, SDTA_ID, PDTA_ID,	/* info/sample/preset */

      IFIL_ID, ISNG_ID, INAM_ID, IROM_ID, /* info ids (1st byte of info strings) */
      IVER_ID, ICRD_ID, IENG_ID, IPRD_ID,	/* more info ids */
      ICOP_ID, ICMT_ID, ISFT_ID,	/* and yet more info ids */

      SNAM_ID, SMPL_ID,		/* sample ids */
      PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,	/* preset ids */
      IHDR_ID, IBAG_ID, IMOD_ID, IGEN_ID,	/* instrument ids */
      SHDR_ID			/* sample info */
      };

/* generator types */
enum Gen_Type {
      Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
      Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_ModLFO2Pitch,
      Gen_VibLFO2Pitch, Gen_ModEnv2Pitch, Gen_FilterFc, Gen_FilterQ,
      Gen_ModLFO2FilterFc, Gen_ModEnv2FilterFc, Gen_EndAddrCoarseOfs,
      Gen_ModLFO2Vol, Gen_Unused1, Gen_ChorusSend, Gen_ReverbSend, Gen_Pan,
      Gen_Unused2, Gen_Unused3, Gen_Unused4,
      Gen_ModLFODelay, Gen_ModLFOFreq, Gen_VibLFODelay, Gen_VibLFOFreq,
      Gen_ModEnvDelay, Gen_ModEnvAttack, Gen_ModEnvHold, Gen_ModEnvDecay,
      Gen_ModEnvSustain, Gen_ModEnvRelease, Gen_Key2ModEnvHold,
      Gen_Key2ModEnvDecay, Gen_VolEnvDelay, Gen_VolEnvAttack,
      Gen_VolEnvHold, Gen_VolEnvDecay, Gen_VolEnvSustain, Gen_VolEnvRelease,
      Gen_Key2VolEnvHold, Gen_Key2VolEnvDecay, Gen_Instrument,
      Gen_Reserved1, Gen_KeyRange, Gen_VelRange,
      Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
      Gen_Attenuation, Gen_Reserved2, Gen_EndLoopAddrCoarseOfs,
      Gen_CoarseTune, Gen_FineTune, Gen_SampleId, Gen_SampleModes,
      Gen_Reserved3, Gen_ScaleTune, Gen_ExclusiveClass, Gen_OverrideRootKey,
      Gen_Dummy
      };

#define Gen_MaxValid 	Gen_Dummy - 1	/* maximum valid generator */

/* generator unit type */
enum Gen_Unit {
      None,				/* No unit type */
      Unit_Smpls,			/* in samples */
      Unit_32kSmpls,		/* in 32k samples */
      Unit_Cent,			/* in cents (1/100th of a semitone) */
      Unit_HzCent,		/* in Hz Cents */
      Unit_TCent,			/* in Time Cents */
      Unit_cB,			/* in centibels (1/100th of a decibel) */
      Unit_Percent,	      /* in percentage */
      Unit_Semitone,		/* in semitones */
      Unit_Range			/* a range of values */
      };

/* global data */

#define CHNKIDSTR(id)           &idlist[(id - 1) * 4]

/* sfont file chunk sizes */
#define SFPHDRSIZE	38
#define SFBAGSIZE	      4
#define SFMODSIZE	      10
#define SFGENSIZE	      4
#define SFIHDRSIZE	22
#define SFSHDRSIZE	46

/* sfont file data structures */

struct SFPhdr {
      unsigned char name[20];		/* preset name */
      unsigned short preset;		/* preset number */
      unsigned short bank;	      /* bank number */
      unsigned short pbagndx;		/* index into preset bag */
      unsigned int library;		/* just for preserving them */
      unsigned int genre;		/* Not used */
      unsigned int morphology;	/* Not used */
      };

struct SFBag {
      unsigned short genndx;		/* index into generator list */
      unsigned short modndx;		/* index into modulator list */
      };

struct SFIhdr {
      char name[20];		      /* Name of instrument */
      unsigned short ibagndx;		/* Instrument bag index */
      };

/* Basic bit swapping functions
 */
#define GUINT16_SWAP_LE_BE_CONSTANT(val)	((unsigned short) ( \
    (((unsigned short) (val) & (unsigned short) 0x00ffU) << 8) | \
    (((unsigned short) (val) & (unsigned short) 0xff00U) >> 8)))
#define GUINT32_SWAP_LE_BE_CONSTANT(val)	((unsigned int) ( \
    (((unsigned int) (val) & (unsigned int) 0x000000ffU) << 24) | \
    (((unsigned int) (val) & (unsigned int) 0x0000ff00U) <<  8) | \
    (((unsigned int) (val) & (unsigned int) 0x00ff0000U) >>  8) | \
    (((unsigned int) (val) & (unsigned int) 0xff000000U) >> 24)))

#define GUINT16_SWAP_LE_BE(val) (GUINT16_SWAP_LE_BE_CONSTANT (val))
#define GUINT32_SWAP_LE_BE(val) (GUINT32_SWAP_LE_BE_CONSTANT (val))

#define GINT16_TO_LE(val)	((signed short) (val))
#define GUINT16_TO_LE(val)	((unsigned short) (val))
#define GINT16_TO_BE(val)	((signed short) GUINT16_SWAP_LE_BE (val))
#define GUINT16_TO_BE(val)	(GUINT16_SWAP_LE_BE (val))
#define GINT32_TO_LE(val)	((signed int) (val))
#define GUINT32_TO_LE(val)	((unsigned int) (val))
#define GINT32_TO_BE(val)	((signed int) GUINT32_SWAP_LE_BE (val))
#define GUINT32_TO_BE(val)	(GUINT32_SWAP_LE_BE (val))

/* The G*_TO_?E() macros are defined in glibconfig.h.
 * The transformation is symmetric, so the FROM just maps to the TO.
 */
#define GINT16_FROM_LE(val)	(GINT16_TO_LE (val))
#define GUINT16_FROM_LE(val)	(GUINT16_TO_LE (val))
#define GINT16_FROM_BE(val)	(GINT16_TO_BE (val))
#define GUINT16_FROM_BE(val)	(GUINT16_TO_BE (val))
#define GINT32_FROM_LE(val)	(GINT32_TO_LE (val))
#define GUINT32_FROM_LE(val)	(GUINT32_TO_LE (val))
#define GINT32_FROM_BE(val)	(GINT32_TO_BE (val))
#define GUINT32_FROM_BE(val)	(GUINT32_TO_BE (val))
}

#endif  /* _FLUID_SFONT_H */
