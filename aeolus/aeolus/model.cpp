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

#include <time.h>
#include "model.h"
#include "scales.h"
#include "global.h"
#include "aeolus.h"

#define VERSION "0.0.0"

//---------------------------------------------------------
//   Divis
//---------------------------------------------------------

Divis::Divis() : _flags(0), _dmask(0), _nrank(0)
      {
      *_label = 0;
      _param[SWELL].set("swell", SWELL_DEF, SWELL_MIN, SWELL_MAX);
      _param[TFREQ].set("tfreq", TFREQ_DEF, TFREQ_MIN, TFREQ_MAX);
      _param[TMODD].set("tmodd", TMODD_DEF, TMODD_MIN, TMODD_MAX);
      }

Keybd::Keybd() : _flags(0)
      {
      *_label = 0;
      }

Ifelm::Ifelm() : _state(0)
      {
      *_label = 0;
      *_mnemo = 0;
      }

Group::Group() : _nifelm(0)
      {
      *_label = 0;
      }

//---------------------------------------------------------
//   Model
//---------------------------------------------------------

Model::Model (Aeolus* a,
   uint16_t* midimap,
   const char* stops,
   const char* instr,
   const char* waves)
   :
   _aeolus(a),
   _midimap (midimap),
   _stops (stops),
   _ready (false),
   _nasect (0),
   _ndivis (0),
   _nkeybd (0),
   _ngroup (0),
   _count (0),
   _bank (0),
   _pres (0),
   _sc_cmode (0),
   _sc_group (0)
      {
      sprintf (_instr, "%s/%s", stops, instr);
      _waves = waves;
      memset (_midimap, 0, 16 * sizeof (uint16_t));
      memset (_preset, 0, NBANK * NPRES * sizeof (Preset *));
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Model::init()
      {
      read_instr ();
      read_presets ();

      init_audio();
      init_iface();
      init_ranks(MT_LOAD_RANK);
      init_ranks(MT_SAVE_RANK);
      }

//---------------------------------------------------------
//   proc_mesg
//---------------------------------------------------------

void Model::proc_mesg (ITC_mesg* /*M*/)
      {
#if 0
      // Handle commands from other threads, including
      // the user interface.

      printf("proc_mesg   ===%ld\n", M->type());

      switch (M->type ()) {
            case MT_IFC_ELCLR:
            case MT_IFC_ELSET:
            case MT_IFC_ELXOR:
                  {
                  // Set, reset or toggle a stop.
                  M_ifc_ifelm *X = (M_ifc_ifelm *) M;
                  set_ifelm (X->_group, X->_ifelm, X->type () - MT_IFC_ELCLR);
                  break;
                  }
      case MT_IFC_ELATT:
            send_event(TO_IFACE, M);
            M = 0;
            break;

    case MT_IFC_GRCLR:
    {
	// Reset a group of stops.
        M_ifc_ifelm *X = (M_ifc_ifelm *) M;
        clr_group (X->_group);
	break;
    }
    case MT_IFC_AUPAR:
    {
	// Set audio section parameter.
        M_ifc_aupar *X = (M_ifc_aupar *) M;
        set_aupar (X->_srcid, X->_asect, X->_parid, X->_value);
	break;
    }
    case MT_IFC_DIPAR:
    {
	// Set division parameter.
        M_ifc_dipar *X = (M_ifc_dipar *) M;
        set_dipar (X->_srcid, X->_divis, X->_parid, X->_value);
	break;
    }
    case MT_IFC_RETUNE:
    {
	// Recalculate all wavetables.
        M_ifc_retune *X = (M_ifc_retune *) M;
        retune (X->_freq, X->_temp);
	break;
    }
    case MT_IFC_ANOFF:
    {
	// All notes off.
	M_ifc_anoff *X = (M_ifc_anoff *) M;
        midi_off (X->_bits);
	break;
    }
    case MT_IFC_MCSET:
    {
	// Store midi routing preset.
        int index;
	M_ifc_chconf *X = (M_ifc_chconf *) M;
        index = X->_index;
        if (index >= 0)
	{
	    if (index >= 8) break;
            memcpy (_chconf [X->_index]._bits, X->_bits, 16 * sizeof (uint16_t));
	}
        set_mconf (X->_index, X->_bits);
    }
    case MT_IFC_MCGET:
    {
	// Recall midi routing preset.
        int index;
	M_ifc_chconf *X = (M_ifc_chconf *) M;
        index = X->_index;
        if (index >= 0)
	{
	    if (index >= 8) break;
            set_mconf (X->_index, _chconf [X->_index]._bits);
	}
	break;
    }
    case MT_IFC_PRRCL:
    {
	// Load a preset.
	M_ifc_preset *X = (M_ifc_preset *) M;
        if (X->_bank < 0) X->_bank = _bank;
        set_state (X->_bank, X->_pres);
        break;
    }
    case MT_IFC_PRDEC:
    {
	// Decrement preset and load.
	if (_pres > 0) set_state (_bank, --_pres);
        break;
    }
    case MT_IFC_PRINC:
    {
	// Increment preset and load.
	if (_pres < NPRES - 1) set_state (_bank, ++_pres);
        break;
    }
    case MT_IFC_PRSTO:
    {
	// Store a preset.
	M_ifc_preset  *X = (M_ifc_preset *) M;
        uint32_t       d [NGROUP];
        get_state (d);
        set_preset (X->_bank, X->_pres, d);
        break;
    }
    case MT_IFC_PRINS:
    {
	// Insert a preset.
	M_ifc_preset *X = (M_ifc_preset *) M;
        uint32_t     d [NGROUP];
        get_state (d);
        ins_preset (X->_bank, X->_pres, d);
        break;
    }
    case MT_IFC_PRDEL:
    {
	// Delete a preset.
	M_ifc_preset *X = (M_ifc_preset *) M;
        del_preset (X->_bank, X->_pres);
        break;
    }
    case MT_IFC_PRGET:
    {
	// Read a preset.
	M_ifc_preset *X = (M_ifc_preset *) M;
        X->_stat = get_preset (X->_bank, X->_pres, X->_bits);
        send_event (TO_IFACE, M);
        M = 0;
        break;
    }
    case MT_IFC_EDIT:
    {
	// Start editing a stop.
	M_ifc_edit *X = (M_ifc_edit *) M;
        Rank       *R = find_rank (X->_group, X->_ifelm);
        if (_ready && R)
	{
            X->_synth = R->_sdef;
            send_event (TO_IFACE, M);
            M = 0;
	}
	break;
    }
    case MT_IFC_APPLY:
    {
	// Apply edited stop.
	M_ifc_edit *X = (M_ifc_edit *) M;
        if (_ready) recalc (X->_group, X->_ifelm);
	break;
    }
    case MT_IFC_SAVE:
	// Save presets, midi presets, and wavetables.
	save ();
        break;

            case MT_LOAD_RANK:
            case MT_CALC_RANK:
                  {
                  // Load a rank into a division.
                  M_def_rank *X = (M_def_rank *) M;
                  _divis [X->_divis]._ranks [X->_rank]._wave = X->_wave;
                  break;
                  }

    case MT_AUDIO_SYNC:
	// Wavetable calculation done.
        send_event (TO_IFACE, new ITC_mesg (MT_IFC_READY));
        _ready = true;
	break;

    default:
        fprintf (stderr, "Model: unexpected message, type = %ld\n", M->type ());
    }
    if (M) M->recover ();
#endif
}


#if 0
void Model::proc_qmidi()
      {
      int c, d, p, t, v;

    // Handle commands from the qmidi queue. These are coming
    // from either the midi thread (ALSA), or the audio thread
    // (JACK). They are encoded as raw MIDI, except that all
    // messages are 3 bytes. All command have already been
    // checked at the sending side.

      while (_qmidi->read_avail () >= 3) {
            t = _qmidi->read (0);
            p = _qmidi->read (1);
            v = _qmidi->read (2);
            _qmidi->read_commit (3);
printf("Midi %x %x %x\n", t, p, v);
            c = t & 0x0F;
            d = (_midimap [c] >>  8) & 7;

            switch (t & 0xF0) {
                  case 0x90:
                        // Program change from notes 24..33.
                        set_state (_bank, p - 24);
                        break;

                  case 0xB0:
                        // Controllers.
                        switch (p) {
                              case MIDICTL_SWELL:
                     		      // Swell pedal
                                    set_dipar (SRC_MIDI_PAR, d, 0, SWELL_MIN + v * (SWELL_MAX - SWELL_MIN) / 127.0f);
                                    break;

                              case MIDICTL_TFREQ:
                                    // Tremulant frequency
                                    set_dipar (SRC_MIDI_PAR, d, 1, TFREQ_MIN + v * (TFREQ_MAX - TFREQ_MIN) / 127.0f);
                                    break;

                              case MIDICTL_TMODD:
                                    // Tremulant amplitude
                                    set_dipar (SRC_MIDI_PAR, d, 2, TMODD_MIN + v * (TMODD_MAX - TMODD_MIN) / 127.0f);
                                    break;

                              case MIDICTL_BANK:
                                    // Preset bank.
                                    if (v < NBANK)
                                          _bank = v;
                                    break;
                              }
                        break;

                  case 0xC0:
	                  // Program change.
	                  if (p < NPRES)
                              set_state (_bank, p);
                        break;
                  }
            }
      }
#endif

void Model::init_audio()
      {
      Divis *D;
      int d;

      for (d = 0, D = _divis; d < _ndivis; d++, D++) {
            M_new_divis* M = new M_new_divis ();
            M->_flags = D->_flags;
            M->_dmask = D->_dmask;
            M->_asect = D->_asect;
            M->_swell = D->_param [Divis::SWELL].fval();
            M->_tfreq = D->_param [Divis::TFREQ].fval();
            M->_tmodd = D->_param [Divis::TMODD].fval();
            _aeolus->newDivis(M);
            }
      }

//---------------------------------------------------------
//   init_iface
//---------------------------------------------------------

void Model::init_iface()
      {
      M_ifc_init* M = new M_ifc_init;
      _aeolus->_ifc_init = M;
      M->_stops  = _stops;
      M->_waves  = _waves;
      M->_instr  = _instr;
      M->_client = 0;     // _midi->_client;
      M->_ipport = 0;     // _midi->_ipport;
      M->_nasect = _nasect;
      M->_nkeybd = _nkeybd;
      M->_ndivis = _ndivis;
      M->_ngroup = _ngroup;
      M->_ntempe = NSCALES;
      for (int i = 0; i < NKEYBD; i++) {
            Keybd* K = _keybd + i;
            M->_keybdd [i]._label = K->_label;
            M->_keybdd [i]._flags = K->_flags;
            }
      for (int i = 0; i < NDIVIS; i++) {
            Divis* D = _divis + i;
            M->_divisd [i]._label = D->_label;
            M->_divisd [i]._flags = D->_flags;
            M->_divisd [i]._asect = D->_asect;
            }
      for (int i = 0; i < NGROUP; i++) {
            Group* G = _group + i;
            M->_groupd [i]._label  = G->_label;
            M->_groupd [i]._nifelm = G->_nifelm;
            for (int j = 0; j < G->_nifelm; j++) {
                  M->_groupd [i]._ifelmd [j]._label = G->_ifelms [j]._label;
                  M->_groupd [i]._ifelmd [j]._mnemo = G->_ifelms [j]._mnemo;
                  M->_groupd [i]._ifelmd [j]._type  = G->_ifelms [j]._type;
                  }
            }
      for (int i = 0; i < NSCALES; i++) {
            M->_temped [i]._label = scales [i]._label;
            M->_temped [i]._mnemo = scales [i]._mnemo;
            }

      set_mconf (0, _chconf[0]._bits);
      }

void Model::init_ranks (int comm)
      {
      _count++;
      _ready = false;
//WS      send_event (TO_IFACE, new M_ifc_retune (_fbase, _itemp));

      for (int g = 0; g < _ngroup; g++) {
            Group* G = _group + g;
            for (int i = 0; i < G->_nifelm; i++)
                  proc_rank (g, i, comm);
            }
      _ready = true;
      }


void Model::proc_rank (int g, int i, int comm)
      {
      Ifelm* I = _group [g]._ifelms + i;
      if ((I->_type == Ifelm::DIVRANK) || (I->_type == Ifelm::KBDRANK)) {
            int d = (I->_action0 >> 16) & 255;
            int r = (I->_action0 >>  8) & 255;
            Rank* R = _divis [d]._ranks + r;
            if (comm == MT_SAVE_RANK) {
                  if (R->_wave->modif ()) {
                        R->_wave->save(_waves, R->_sdef, _aeolus->_fsamp,
                           _fbase, scales[_itemp]._data);
                        }
                  }
            else if (R->_count != _count) {
                  R->_count = _count;
                  M_def_rank* M = new M_def_rank (comm);
                  M->_divis = d;
                  M->_rank  = r;
                  M->_group = g;
                  M->_ifelm = i;
                  M->_fsamp = _aeolus->_fsamp;
                  M->_fbase = _fbase;
                  M->_scale = scales [_itemp]._data;
                  M->_sdef  = R->_sdef;
                  M->_wave  = R->_wave;
                  M->_path  = _waves;

//WS                  send_event(TO_IFACE, new M_ifc_ifelm (MT_IFC_ELATT, M->_group, M->_ifelm));

                  M->_wave = new Rankwave (M->_sdef->_n0, M->_sdef->_n1);
                  if (M->_wave->load (M->_path, M->_sdef, M->_fsamp, M->_fbase, M->_scale))
                        M->_wave->gen_waves (M->_sdef, M->_fsamp, M->_fbase, M->_scale);

                  _aeolus->_divisp [M->_divis]->set_rank (M->_rank, M->_wave,  M->_sdef->_pan, M->_sdef->_del);
                  _divis [M->_divis]._ranks [M->_rank]._wave = M->_wave;
                  }
            }
      }

//---------------------------------------------------------
//   set_ifelm
//    Set, reset or toggle a stop.
//          m - 0 reset
//              1 set
//              2 toggle
//---------------------------------------------------------

void Model::set_ifelm (int g, int i, int m)
      {
      Group* G = _group + g;

      if ((!_ready) || (g >= _ngroup) || (i >= G->_nifelm)) {
            printf("Aeolus::Model::set_ifelm failed\n");
            return;
            }
      Ifelm* I = G->_ifelms + i;
      int s = (m == 2) ? I->_state ^ 1 : m;
      if (I->_state != s) {
            I->_state = s;
            _aeolus->proc_queue(s ? I->_action1 : I->_action0);
            M_ifc_ifelm*  e = new M_ifc_ifelm (MT_IFC_ELCLR + s, g, i);
            if (m == 0)
                  _aeolus->_ifelms [e->_group] &= ~(1 << e->_ifelm);
            else if (m == 1)
                  _aeolus->_ifelms [e->_group] |= (1 << e->_ifelm);
            else if (m == 2)
                  _aeolus->_ifelms [e->_group] = 0;
            }
      }

void Model::clr_group (int g)
      {
      Group* G = _group + g;

      if ((! _ready) || (g >= _ngroup))
            return;

      for (int i = 0; i < G->_nifelm; i++) {
            Ifelm* I = G->_ifelms + i;
            if (I->_state) {
                  I->_state = 0;
                  _aeolus->proc_queue(I->_action0);
                  }
            }
      _aeolus->_ifelms[g] = 0;
      }


//---------------------------------------------------------
//   get_state
//---------------------------------------------------------

void Model::get_state(uint32_t* d)
      {
      for (int g = 0; g < _ngroup; g++) {
            Group* G = _group + g;
            uint32_t s = 0;
            for (int i = 0; i < G->_nifelm; i++) {
                  Ifelm* I = G->_ifelms + i;
                  if (I->_state & 1)
                        s |= 1 << i;
                  }
            *d++ = s;
            }
      }


//---------------------------------------------------------
//   set_state
//---------------------------------------------------------

void Model::set_state(int bank, int pres)
      {
      uint32_t d[NGROUP];

      _bank = bank;
      _pres = pres;
      if (get_preset(bank, pres, d)) {
            for (int g = 0; g < _ngroup; g++) {
                  uint32_t s = d [g];
                  Group* G = _group + g;
                  for (int i = 0; i < G->_nifelm; i++) {
                        set_ifelm (g, i, s & 1);
                        s >>= 1;
                        }
                  }
//WS        send_event (TO_IFACE, new M_ifc_preset (MT_IFC_PRRCL, bank, pres, _ngroup, d));
            }
      else  {
//WS        send_event (TO_IFACE, new M_ifc_preset (MT_IFC_PRRCL, bank, pres, 0, 0));
            }
      }


void Model::set_aupar (int /*s*/, int a, int p, float v)
      {
      SyntiParameter* P = ((a < 0) ? _aeolus->_audio->_instrpar : _aeolus->_audio->_asectpar [a]) + p;
      if (v < P->min())
            v = P->min();
      if (v > P->max())
            v = P->max();
      P->set(v);
//WS    send_event (TO_IFACE, new M_ifc_aupar (s, a, p, v));
      }


void Model::set_dipar (int /*s*/, int d, int p, float v)
{
    SyntiParameter  *P;
//    union { uint32_t i; float f; } u;

    P = _divis [d]._param + p;
    if (v < P->min())
            v = P->min();
    if (v > P->max())
            v = P->max();
    P->set(v);
printf("Model::set_dipar\n");
#if 0
    if (_qcomm->write_avail () >= 2)
    {
	u.f = v;
	_qcomm->write (0, (17 << 24) | (d << 16) | (p << 8));
	_qcomm->write (1, u.i);
        _qcomm->write_commit (2);
//WS        send_event (TO_IFACE, new M_ifc_dipar (s, d, p, v));
    }
#endif
}


void Model::set_mconf (int /*i*/, uint16_t *d)
      {
      midi_off(127);
      for (int j = 0; j < 16; j++) {
            int a = d [j];
            int b =  (a & 0x1000) ? (_keybd [a & 7]._flags & 127) : 0;
            b |= a & 0x7700;
            _midimap [j] = b;
            }
//WS    send_event (TO_IFACE, new M_ifc_chconf (MT_IFC_MCSET, i, d));
      }


void Model::midi_off (int mask)
      {
      mask &= 127;
      _aeolus->proc_queue((2 << 24) | (mask << 16) | mask);
      }


void Model::retune (float freq, int temp)
{
    if (_ready)
    {
	_fbase = freq;
        _itemp = temp;
        init_ranks (MT_CALC_RANK);
    }
    else  {
//WS            send_event (TO_IFACE, new M_ifc_retune (_fbase, _itemp));
            }
}

void Model::recalc (int g, int i)
      {
      _count++;
      _ready = false;
      proc_rank (g, i, MT_CALC_RANK);
      }

void Model::save ()
      {
      write_instr ();
      writePresets();
      _ready = false;
      for (int g = 0; g < _ngroup; g++) {
            Group* G = _group + g;
            for (int i = 0; i < G->_nifelm; i++)
                  proc_rank (g, i, MT_SAVE_RANK);
            }
      }

Rank *Model::find_rank (int g, int i)
      {
      Ifelm* I = _group [g]._ifelms + i;
      if ((I->_type == Ifelm::DIVRANK) || (I->_type == Ifelm::KBDRANK)) {
            int d = (I->_action0 >> 16) & 255;
            int r = (I->_action0 >>  8) & 255;
            return _divis [d]._ranks + r;
            }
      return 0;
      }


int Model::read_instr ()
      {
      int           line, stat, n;
      bool          instr;
      int           d, k, r, s;
      char          c, *p, *q;
      char          buff [1024];
      char          t1 [256];
      char          t2 [256];
      Keybd         *K;
      Divis         *D;
      Rank          *R;
      Group         *G;
      Ifelm         *I;
      Addsynth      *A;

      enum { CONT, DONE, ERROR, COMM, ARGS, MORE, NO_INSTR, IN_INSTR,
           BAD_SCOPE, BAD_ASECT, BAD_RANK, BAD_DIVIS, BAD_KEYBD, BAD_IFACE,
           BAD_STR1, BAD_STR2 };

      QFile f(QString("%1/definition").arg(_instr));
      if (!f.open(QIODevice::ReadOnly))  {
            fprintf (stderr, "Can't open '%s' for reading\n", qPrintable(f.fileName()));
            return 1;
            }

      stat = 0;
      line = 0;
      instr = false;
      D = 0;
      G = 0;
      d = k = r = s = 0;
#ifdef WIN32
      QByteArray bb;
      while (!stat) {
            line++;
            bb = f.readLine(1024);
            if(bb.isEmpty())
                  break;
            p = bb.data();
#else
      while (!stat && f.readLine(buff, 1024)) {
            line++;
            p = buff;
#endif
            if (*p != '/') {
                  while (isspace (*p))
                        p++;
                  if ((*p > ' ') && (*p != '#')) {
                        fprintf (stderr, "Syntax error in line %d\n", line);
                        stat = COMM;
                        }
                  continue;
                  }

            q = p;
            while ((*q >= ' ') && !isspace (*q))
                  q++;
            *q++ = 0;
            while ((*q >= ' ') && isspace (*q))
                  q++;

            if (! strcmp (p, "/instr/new")) {
                  if (instr)
                        stat = IN_INSTR;
                  else
                        instr = true;
                  }
            else if (! instr) {
                  stat = NO_INSTR;
                  }
            else if (! strcmp (p, "/instr/end")) {
                  instr = false;
                  stat = DONE;
                  }
            else if (! strcmp (p, "/manual/new") || ! strcmp (p, "/pedal/new")) {
                  if (D || G)
                        stat = BAD_SCOPE;
                  else if (sscanf (q, "%s%n", t1, &n) != 1)
                        stat = ARGS;
                  else {
		            q += n;
                        if (_nkeybd == NKEYBD) {
                              fprintf (stderr, "Line %d: can't create more than %d keyboards\n", line, NKEYBD);
                              stat = ERROR;
                              }
                        else if (strlen (t1) > 15)
                              stat = BAD_STR1;
                        else {
                              k = _nkeybd++;
                              K = _keybd + k;
                              strcpy (K->_label, t1);
                              K->_flags = 1 << k;
                              if (p [1] == 'p')
                                    K->_flags |= HOLD_MASK | Keybd::IS_PEDAL;
                              }
                        }
                  }
            else if (! strcmp (p, "/divis/new")) {
                  if (D || G)
                        stat = BAD_SCOPE;
                  else if (sscanf (q, "%s%d%d%n", t1, &k, &s, &n) != 3)
                        stat = ARGS;
                  else
	    {
		q += n;
		if (_ndivis == NDIVIS)
		{
		    fprintf (stderr, "Line %d: can't create more than %d divisions\n", line, NDIVIS);
		    stat = ERROR;
		}
		else if (strlen (t1) > 15) stat = BAD_STR1;
		else if ((k < 0) || (k > _nkeybd)) stat = BAD_KEYBD;
		else if ((s < 1) || (s > NASECT))  stat = BAD_ASECT;
		else
		{
		    D = _divis + _ndivis++;
		    strcpy (D->_label, t1);
                    if (_nasect < s) _nasect = s;
		    D->_asect = s - 1;
                    D->_keybd = k - 1;
		    if (k--) D->_dmask = _keybd [k]._flags & 127;
		}
	    }
	}
        else if (! strcmp (p, "/divis/end"))
        {
	    if (!D || G) stat = BAD_SCOPE;
            else D = 0;
	}
        else if (! strcmp (p, "/group/new"))
        {
	    if (D || G) stat = BAD_SCOPE;
            else if (sscanf (q, "%s%n", t1, &n) != 1) stat = ARGS;
            else
	    {
		q += n;
		if (_ngroup == NGROUP)
		{
		    fprintf (stderr, "Line %d: can't create more than %d groups\n", line, NGROUP);
		    stat = ERROR;
		}
		else if (strlen (t1) > 15) stat = BAD_STR1;
		else
		{
		    G = _group + _ngroup++;
		    strcpy (G->_label, t1);
		}
	    }
	}
        else if (! strcmp (p, "/group/end"))
        {
	    if (D || !G) stat = BAD_SCOPE;
            else G = 0;
	}

            else if (! strcmp (p, "/tuning")) {
                  if (D || G)
                        stat = BAD_SCOPE;
                  else {
                        int nargs = sscanf (q, "%f %d%n", &_fbase, &_itemp, &n);
                        if (nargs != 2)
                              stat = ARGS;
                        else
                              q += n;
                        }
                  }

        else if (! strcmp (p, "/rank"))
        {
	    if (!D && G) stat = BAD_SCOPE;
            else if (sscanf (q, "%c%d%s%n", &c, &d, t1, &n) != 3) stat = ARGS;
            else
	    {
   	        q += n;
                if (D->_nrank == Divis::NRANK)
		{
		    fprintf (stderr, "Line %d: can't create more than %d ranks per division\n", line, Divis::NRANK);
		    stat = ERROR;
		}
                else if (strlen (t1) > 63) stat = BAD_STR1;
                else
		{
                    A = new Addsynth;
        	    strcpy (A->_filename, t1);
                    if (A->load (_stops))
		    {
			stat = ERROR;
			delete A;
 		    }
                    else
		    {
                        A->_pan = c;
                        A->_del = d;
			R = D->_ranks + D->_nrank++;
                        R->_count = 0;
                        R->_sdef = A;
                        R->_wave = 0;
		    }
 		}
	    }
	}
        else if (! strcmp (p, "/tremul"))
        {
	    if (D)
	    {
            float val1;
            float val2;
		if (sscanf (q, "%f%f%n", &val1, &val2, &n) != 2)
               stat = ARGS;
            else {
                D->_param[Divis::TFREQ].set(val1);
                D->_param[Divis::TMODD].set(val2);
		    q += n;
		    D->_flags |= Divis::HAS_TREM;
		}
	    }
	    else if (G)
	    {
		if (sscanf (q, "%d%s%s%n", &d, t1, t2, &n) != 3) stat = ARGS;
                else
		{
		    q += n;
                    if (G->_nifelm == Group::NIFELM) stat = BAD_IFACE;
                    else if ((d < 1) || (d > _ndivis)) stat = BAD_DIVIS;
	   	    else if (strlen (t1) >  7) stat = BAD_STR1;
		    else if (strlen (t2) > 31) stat = BAD_STR2;
                    else
		    {
                        d--;
			I = G->_ifelms + G->_nifelm++;
			strcpy (I->_mnemo, t1);
			strcpy (I->_label, t2);
			I->_type = Ifelm::TREMUL;
			I->_action0 = (16 << 24) | (d << 16) | 0;
			I->_action1 = (16 << 24) | (d << 16) | 1;
		    }
		}
	    }
            else stat = BAD_SCOPE;
	}
        else if (! strcmp (p, "/swell"))
        {
	    if (!D || G) stat = BAD_SCOPE;
            else D->_flags |= Divis::HAS_SWELL;
	}
        else if (! strcmp (p, "/stop"))
        {
	    if (D || !G) stat = BAD_SCOPE;
            else if (sscanf (q, "%d%d%d%n", &k, &d, &r, &n) != 3) stat = ARGS;
            else
	    {
    	        q += n;
                if (G->_nifelm == Group::NIFELM) stat = BAD_IFACE;
                else if ((k < 0) || (k > _nkeybd)) stat = BAD_KEYBD;
                else if ((d < 1) || (d > _ndivis)) stat = BAD_DIVIS;
                else if ((r < 1) || (r > _divis [d - 1]._nrank)) stat = BAD_RANK;
                else
		{
                    k--;
                    d--;
                    r--;
		    I = G->_ifelms + G->_nifelm++;
                    R = _divis [d]._ranks + r;
                    strcpy (I->_label, R->_sdef->_stopname);
                    strcpy (I->_mnemo, R->_sdef->_mnemonic);
                    I->_keybd = k;
                    if (k >= 0)
		    {
                        I->_type = Ifelm::KBDRANK;
			k = _keybd [k]._flags & 127;
		    }
		    else
		    {
                        I->_type = Ifelm::DIVRANK;
			k = 128;
		    }
                    I->_action0 = (6 << 24) | (d << 16) | (r << 8) | k;
                    I->_action1 = (7 << 24) | (d << 16) | (r << 8) | k;
    		}
	    }
	}
        else if (! strcmp (p, "/coupler"))
        {
	    if (D || !G) stat = BAD_SCOPE;
            else if (sscanf (q, "%d%d%s%s%n", &k, &d, t1, t2, &n) != 4) stat = ARGS;
            else
	    {
		q += n;
                if (G->_nifelm == Group::NIFELM) stat = BAD_IFACE;
                else if ((k < 1) || (k > _nkeybd)) stat = BAD_KEYBD;
                else if ((d < 1) || (d > _ndivis)) stat = BAD_DIVIS;
		else if (strlen (t1) >  7) stat = BAD_STR1;
		else if (strlen (t2) > 31) stat = BAD_STR2;
                else
		{
                    k--;
                    d--;
		    I = G->_ifelms + G->_nifelm++;
                    strcpy (I->_mnemo, t1);
                    strcpy (I->_label, t2);
                    I->_type = Ifelm::COUPLER;
	            I->_keybd = k;
                    k = _keybd [k]._flags & 127;
                    I->_action0 = (4 << 24) | (d << 16) | k;
                    I->_action1 = (5 << 24) | (d << 16) | k;
		}
	    }
	}
        else stat = COMM;

        if (stat <= DONE)
	{
            while (isspace (*q)) q++;
            if (*q > ' ') stat = MORE;
	}

        switch (stat)
	{
        case COMM:
	    fprintf (stderr, "Line %d: unknown command '%s'\n", line, p);
            break;
        case ARGS:
	    fprintf (stderr, "Line %d: missing arguments in '%s' command\n", line, p);
            break;
        case MORE:
	    fprintf (stderr, "Line %d: extra arguments in '%s' command\n", line, p);
            break;
        case NO_INSTR:
	    fprintf (stderr, "Line %d: command '%s' outside instrument scope\n", line, p);
            break;
        case IN_INSTR:
	    fprintf (stderr, "Line %d: command '%s' inside instrument scope\n", line, p);
            break;
        case BAD_SCOPE:
	    fprintf (stderr, "Line %d: command '%s' in wrong scope\n", line, p);
            break;
        case BAD_ASECT:
	    fprintf (stderr, "Line %d: no section '%d'\n", line, s);
            break;
        case BAD_RANK:
	    fprintf (stderr, "Line %d: no rank '%d' in division '%d'\n", line, r, d);
            break;
        case BAD_KEYBD:
	    fprintf (stderr, "Line %d: no keyboard '%d'\n", line, k);
            break;
        case BAD_DIVIS:
	    fprintf (stderr, "Line %d: no division '%d'\n", line, d);
            break;
        case BAD_IFACE:
	    fprintf (stderr, "Line %d: can't create more than '%d' elements per group\n", line, Group::NIFELM);
            break;
        case BAD_STR1:
	    fprintf (stderr, "Line %d: string '%s' is too long\n", line, t1);
            break;
        case BAD_STR2:
	    fprintf (stderr, "Line %d: string '%s' is too long\n", line, t1);
            break;
	}
    }

      f.close();
      return (stat <= DONE) ? 0 : 2;
      }


int Model::write_instr()
      {
      FILE          *F;
      int           d, g, i, k, r;
      char          buff [1024];
      time_t        t;
      Divis         *D;
      Rank          *R;
      Group         *G;
      Ifelm         *I;
      Addsynth      *A;

    sprintf (buff, "%s/definition", _instr);
    if (! (F = fopen (buff, "w")))
    {
	fprintf (stderr, "Can't open '%s' for writing\n", buff);
        return 1;
    }
    printf ("Writing '%s'\n", buff);
    t = time (0);

    fprintf (F, "# Aeolus instrument definition file\n");
    fprintf (F, "# Created by Aeolus-%s at %s\n", VERSION, ctime (&t));

    fprintf (F, "\n/instr/new\n");
    fprintf (F, "/tuning %5.1f %d\n", _fbase, _itemp);

    fprintf (F, "\n# Keyboards\n#\n");
    for (k = 0; k < _nkeybd; k++)
    {
	if (_keybd [k]._flags & Keybd::IS_PEDAL) fprintf (F, "/pedal/new    %s\n", _keybd [k]._label);
        else                                     fprintf (F, "/manual/new   %s\n", _keybd [k]._label);
    }

    fprintf (F, "\n# Divisions\n#\n");
    for (d = 0; d < _ndivis; d++)
    {
	D = _divis + d;
        fprintf (F, "/divis/new    %-7s  %d  %d\n", D->_label, D->_keybd + 1, D->_asect + 1);
        for (r = 0; r < D->_nrank; r++)
	{
            R = D->_ranks + r;
	    A = R->_sdef;
            fprintf (F, "/rank         %c %3d  %s\n", A->_pan, A->_del, A->_filename);
	}
        if (D->_flags & Divis::HAS_SWELL) fprintf (F, "/swell\n");
        if (D->_flags & Divis::HAS_TREM) fprintf (F, "/tremul       %3.1f  %3.1f\n",
                                                  D->_param [Divis::TFREQ].fval(), D->_param [Divis::TMODD].fval());
        fprintf (F, "/divis/end\n\n");
    }

    fprintf (F, "# Interface groups\n#\n");
    for (g = 0; g < _ngroup; g++)
    {
	G = _group + g;
        fprintf (F, "/group/new    %-7s\n", G->_label);
        for (i = 0; i < G->_nifelm; i++)
	{
	    I = G->_ifelms + i;
            switch (I->_type)
	    {
	    case Ifelm::DIVRANK:
	    case Ifelm::KBDRANK:
                k = I->_keybd;
                d = (I->_action0 >> 16) & 255;
                r = (I->_action0 >>  8) & 255;
                fprintf (F, "/stop         %d   %d  %2d\n", k + 1, d + 1, r + 1);
		break;

	    case Ifelm::COUPLER:
                k = I->_keybd;
                d = (I->_action0 >> 16) & 255;
                fprintf (F, "/coupler      %d   %d   %-7s  %s\n", k + 1, d + 1, I->_mnemo, I->_label);
		break;

	    case Ifelm::TREMUL:
                d = (I->_action0 >> 16) & 255;
                D = _divis + d;
                fprintf (F, "/tremul       %d       %-7s  %s\n", d + 1, I->_mnemo, I->_label);
		break;
	    }
	}
        fprintf (F, "/group/end\n\n");
    }

    fprintf (F, "\n/instr/end\n");
    fclose (F);

    return 0;
}

//---------------------------------------------------------
//   get_preset
//---------------------------------------------------------

int Model::get_preset (int bank, int pres, uint32_t *bits)
      {
      if ((bank < 0) | (pres < 0) || (bank >= NBANK) || (pres >= NPRES))
            return 0;
      Preset* P = _preset [bank][pres];
      if (P) {
            for (int k = 0; k < _ngroup; k++)
                  *bits++ = P->_bits [k];
            return _ngroup;
            }
      return 0;
      }

//---------------------------------------------------------
//   set_preset
//---------------------------------------------------------

void Model::set_preset (int bank, int pres, uint32_t *bits)
      {
      if ((bank  < 0) | (pres < 0) || (bank >= NBANK) || (pres >= NPRES))
            return;
      Preset* P = _preset [bank][pres];
      if (!P) {
            P = new Preset;
            _preset [bank][pres] = P;
            }
      for (int k = 0; k < _ngroup; k++)
            P->_bits [k] = *bits++;
      }

//---------------------------------------------------------
//   ins_preset
//---------------------------------------------------------

void Model::ins_preset (int bank, int pres, uint32_t *bits)
      {
      if ((bank < 0) | (pres < 0) || (bank >= NBANK) || (pres >= NPRES))
            return;
      Preset* P = _preset [bank][NPRES - 1];
      for (int j = NPRES - 1; j > pres; j--)
            _preset [bank][j] = _preset [bank][j - 1];
      if (!P) {
            P = new Preset;
            _preset [bank][pres] = P;
            }
      for (int k = 0; k < _ngroup; k++)
            P->_bits [k] = *bits++;
      }

//---------------------------------------------------------
//   del_preset
//---------------------------------------------------------

void Model::del_preset (int bank, int pres)
      {
      if ((bank < 0) | (pres < 0) || (bank >= NBANK) || (pres >= NPRES))
            return;
      delete _preset [bank][pres];
      for (int j = pres; j < NPRES - 1; j++)
            _preset [bank][j] = _preset [bank][j + 1];
      _preset [bank][NPRES - 1] = 0;
      }

//---------------------------------------------------------
//   read_presets
//---------------------------------------------------------

int Model::read_presets()
      {
      char name [1024];
      uchar data [256];

      QFile f(QString("%1/presets").arg(_instr));
      if (!f.open(QIODevice::ReadOnly)) {
            fprintf (stderr, "Can't open '%s' for reading\n", qPrintable(f.fileName()));
            return 1;
            }

      f.read((char*)data, 16);
      if (strcmp ((char *) data, "PRESET") || data [7]) {
            fprintf (stderr, "File '%s' is not a valid preset file\n", name);
            f.close();
            return 1;
            }
      int n = RD2 (data + 14);

      if (f.read ((char*)data, 256) != 256) {
            fprintf (stderr, "No valid data in file '%s'\n", name);
            f.close();
            return 1;
            }
      uchar* p = data;
      for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 16; j++) {
                  _chconf [i]._bits [j] = RD2 (p);
                  p += 2;
                  }
            }
      if (n != _ngroup) {
            fprintf (stderr, "Presets in file '%s' are not compatible\n", name);
            f.close();
            return 1;
            }
      while (f.read ((char*)data, 4 + 4 * _ngroup) == 4 + 4 * _ngroup) {
            p = data;
            int i = *p++;
            int j = *p++;
            p++;
            p++;
            if ((i < NBANK) && (j < NPRES)) {
                  Preset* P = new Preset;
                  for (int k = 0; k < _ngroup; k++) {
                        P->_bits [k] = RD4 (p);
                        p += 4;
                        }
                  _preset [i][j] = P;
                  }
            }
      f.close();
      return 0;
      }

//---------------------------------------------------------
//   write_presets
//---------------------------------------------------------

bool Model::writePresets()
      {
      char   name [1024];
      uchar  data [256];
      FILE   *F;

      sprintf (name, "%s/presets", _instr);
      if (! (F = fopen (name, "w"))) {
            fprintf (stderr, "Can't open '%s' for writing\n", name);
            return 1;
            }

      strcpy ((char *) data, "PRESET");
      data [7] = 0;
      WR2 (data +  8, 0);
      WR2 (data + 10, 0);
      WR2 (data + 12, 0);
      WR2 (data + 14, _ngroup);
      fwrite (data, 16, 1, F);

      uchar* p = data;
      for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 16; j++) {
                  int v = _chconf [i]._bits [j];
                  WR2 (p, v);
                  p += 2;
                  }
            }
      fwrite (data, 256, 1, F);

      for (int i = 0; i < NBANK; i++) {
            for (int j = 0; j < NPRES; j++) {
                  Preset* P = _preset [i][j];
                  if (P) {
                        p = data;
                        *p++ = i;
                        *p++ = j;
                        *p++ = 0;
                        *p++ = 0;
                        for (int k = 0; k < _ngroup; k++) {
                              int v = P->_bits [k];
                              WR4 (p, v);
                              p += 4;
                              }
                        fwrite (data, 4 + 4 * _ngroup, 1, F);
                        }
                  }
            }
      fclose (F);
      return 0;
      }

