/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

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


#ifndef __MODEL_H
#define __MODEL_H


#include "messages.h"
#include "addsynth.h"
#include "rankwave.h"
#include "global.h"

class Aeolus;

class Asect
{
public:

    Asect (void) { *_label = 0; }

    char    _label [64];
};


class Rank
{
public:

    int         _count;
    Addsynth   *_sdef;
    Rankwave   *_wave;
};


class Divis
      {
   public:

      enum { HAS_SWELL = 1, HAS_TREM = 2, NRANK = 32 };
      enum { SWELL, TFREQ, TMODD, NPARAM };

      Divis();

      char        _label [16];
      int         _flags;
      int         _dmask;
      int         _nrank;
      int         _asect;
      int         _keybd;
      SyntiParameter _param [NPARAM];
      Rank        _ranks [NRANK];
      };

class Keybd
{
public:

    enum { IS_PEDAL = 256 };

    Keybd ();

    char    _label [16];
    int     _flags;
};


class Ifelm
{
public:

    enum { DIVRANK, KBDRANK, COUPLER, TREMUL };

    Ifelm ();

    char      _label [32];
    char      _mnemo [8];
    int       _type;
    int       _keybd;
    int       _state;
    uint32_t  _action0;
    uint32_t  _action1;
};

class Group
      {
   public:

      enum { NIFELM = 32 };

      Group();

      char     _label [16];
      int      _nifelm;
      Ifelm    _ifelms [NIFELM];
      };

class Chconf
      {
   public:

      Chconf () { memset (_bits, 0, 16 * sizeof (uint16_t)); }

      uint16_t  _bits [16];
      };

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

class Preset
      {
   public:

      Preset () { memset (_bits, 0, NGROUP * sizeof (uint32_t)); }

      uint32_t  _bits [NGROUP];
      };

//---------------------------------------------------------
//   Model
//---------------------------------------------------------

class Model
      {
      Aeolus*        _aeolus;
      uint16_t*      _midimap;
      const char*    _stops;
      char           _instr [1024];
      const char*    _waves;
      bool           _ready;

      Asect           _asect [NASECT];
      Keybd           _keybd [NKEYBD];
      Divis           _divis [NDIVIS];
      Group           _group [NGROUP];

      int             _nasect;
      int             _ndivis;
      int             _nkeybd;
      int             _ngroup;
      float           _fbase;
      int             _itemp;
      int             _count;
      int             _bank;
      int             _pres;
      int             _client;
      int             _portid;
      int             _sc_cmode; // stop control command mode
      int             _sc_group; // stop control group number
      Chconf          _chconf [8];
      Preset*         _preset [NBANK][NPRES];

      void init_audio();
      void init_iface();
      void init_ranks(int comm);
      void proc_rank(int g, int i, int comm);
      void set_mconf(int i, uint16_t *d);
      void get_state(uint32_t *bits);
      void set_state(int bank, int pres);
      void midi_off(int mask);
      void retune(float freq, int temp);
      void recalc(int g, int i);
      void save();
      Rank* find_rank(int g, int i);
      int  read_instr();
      int  write_instr();
      int  get_preset(int bank, int pres, uint32_t *bits);
      void set_preset(int bank, int pres, uint32_t *bits);
      void ins_preset(int bank, int pres, uint32_t *bits);
      void del_preset(int bank, int pres);
      int  read_presets();
      bool writePresets();

   public:
      Model (Aeolus* aeolus, uint16_t* midimap, const char* stops,
         const char* instr, const char* waves);

      virtual ~Model() {}

      void set_ifelm (int g, int i, int m);
      void clr_group (int g);
      void init ();
      };

#endif

