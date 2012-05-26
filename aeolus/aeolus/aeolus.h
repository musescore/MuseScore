//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __AEOLUS_H__
#define __AEOLUS_H__

struct MidiPatch;
class Event;

#include "stdint.h"
#include "msynth/synti.h"
#include "libmscore/midipatch.h"

#include "asection.h"
#include "division.h"
#include "reverb.h"
#include "global.h"

class Model;
class M_audio_info;
class M_new_divis;
class M_ifc_init;

//---------------------------------------------------------
//   Synth
//---------------------------------------------------------

class Aeolus : public Synth {
      Model* model;
      QList<MidiPatch*> patchList;
      uint16_t _midimap [16];
      int _sc_cmode;          // stop control command mode
      int _sc_group;          // stop control group number

      enum { VOLUME, REVSIZE, REVTIME, STPOSIT };

      volatile bool   _running;
      int             _hold;
      int             _nplay;
      unsigned int    _fsamp;
      int             _nasect;
      int             _ndivis;
      Asection       *_asectp [NASECT];

      Division*       _divisp [NDIVIS];
      Reverb          _reverb;
      unsigned char   _keymap [NNOTES];
      SyntiParameter  _audiopar[4];
      float           _revsize;
      float           _revtime;

      int nout;
      float routb[PERIOD];
      float loutb[PERIOD];

      M_audio_info* _audio;
      M_ifc_init*   _ifc_init;
      uint32_t      _ifelms [NGROUP];
      char          _tempstr[64];

      void proc_synth(int);
      void cond_key_off (int m, int b);
      void cond_key_on (int m, int b);

      void  audio_init(int sampleRate);
      void  audio_start();

      void key_off (int n, int b);
      void key_on (int n, int b);
      void newDivis(M_new_divis* X);
      void proc_queue(uint32_t);
      void printGui();
      void rewrite_label(const char*);

   public:
      Aeolus();
      virtual ~Aeolus();
      virtual void init(int sampleRate);

      virtual const char* name() const { return "Aeolus"; }

      virtual void setMasterTuning(double);
      virtual double masterTuning() const;

      virtual bool loadSoundFonts(const QStringList&) { return true; }
      virtual QStringList soundFonts() const { return QStringList(); }

      virtual void process(unsigned, float*, float);
      virtual void play(const Event&);

      virtual const QList<MidiPatch*>& getPatchInfo() const;

      // set/get a single parameter
      virtual SyntiParameter parameter(int id) const;
      virtual void setParameter(int id, double val);

      // get/set synthesizer state
      virtual SyntiState state() const;
      virtual void setState(const SyntiState&);

      virtual void allSoundsOff(int);
      virtual void allNotesOff(int);

      friend class Model;
      };


enum {
      AEOLUS_VOLUME, AEOLUS_REVSIZE, AEOLUS_REVTIME, AEOLUS_STPOSIT
      };
#endif


