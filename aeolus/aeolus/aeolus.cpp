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

#include "aeolus.h"
#include "model.h"

extern QString dataPath;
extern QString mscoreGlobalShare;

#include "libmscore/event.h"
#include "xml.h"
#include "msynth/sparm_p.h"

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Aeolus::init(int samplerate)
      {
      setlocale(LC_ALL, "C"); // scanf of floats does not work otherwise

      QString stops = mscoreGlobalShare + "/sound/aeolus/stops";
      int n = strlen(qPrintable(stops));
      char* stopsPath = new char[n+1];
      strcpy(stopsPath, qPrintable(stops));

      QDir dir;
      QString waves = dataPath + QString("/aeolus/waves%1").arg(samplerate);
      dir.mkpath(waves);
      n = strlen(qPrintable(waves));
      char* wavesPath = new char[n+1];
      strcpy(wavesPath, qPrintable(waves));

      audio_init(samplerate);
      model = new Model (this, _midimap, stopsPath, "Aeolus", wavesPath);

      audio_start();
      model->init();
//      printGui();
      }

//---------------------------------------------------------
//   Aeolus
//---------------------------------------------------------

Aeolus::Aeolus()
      {
      model = 0;
      MidiPatch* patch = new MidiPatch;
      patch->drum = false;
      patch->bank = 0;
      patch->prog = 0;
      patch->name = "Aeolus";
      patchList.append(patch);
      _sc_cmode = 0;
      _sc_group = 0;
      _audio = new M_audio_info;
      _running = false;
      _nplay = 0;
      _fsamp = 0;
      _nasect = 0;
      _ndivis = 0;
      nout = 0;
      _ifc_init = 0;
      for (int i = 0; i < NGROUP; i++)
            _ifelms [i] = 0;
      memset(_keymap, 0, sizeof(_keymap));
      }

Aeolus::~Aeolus()
      {
      delete model;
      for (int i = 0; i < _nasect; i++)
            delete _asectp [i];
      for (int i = 0; i < _ndivis; i++)
            delete _divisp [i];
      _reverb.fini ();
      }

//---------------------------------------------------------
//   setMasterTuning
//---------------------------------------------------------

void Aeolus::setMasterTuning(double)
      {
      }

//---------------------------------------------------------
//   masterTuning
//---------------------------------------------------------

double Aeolus::masterTuning() const
      {
      return 440.0;
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Aeolus::play(const Event& event)
      {
      int ch   = event.channel();
      int type = event.type();
      int m    = _midimap [ch] & 127;        // Keyboard and hold bits
// printf("Aeolus::play %d %d %d\n", ch, type, m);

//      int f    = (_midimap [ch] >> 12) & 7;  // Control enabled if (f & 4)
      if (type == ME_NOTEON) {
            int n = event.dataA();
            int v = event.dataB();
            if (v == 0) {   // note off
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        key_off(n - 36, m);
                  }
            else {            // note on
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        key_on(n - 36, m);
                  }
            }
      else if (type == ME_CONTROLLER) {
            int p = event.dataA();
            int v = event.dataB();
            switch(p) {
                  case MIDICTL_HOLD:
                  case MIDICTL_ASOFF:
                  case MIDICTL_ANOFF:
                        break;
                  case MIDICTL_BANK:
                        break;
                  case MIDICTL_IFELM:
                        // if (!(f & 4))  enabale control for all channels
                        //      break;
                        if (v & 64) {
                              // Set mode or clear group.
                              _sc_cmode = (v >> 4) & 3;
                              _sc_group = v & 7;
                              if (_sc_cmode == 0) {
                                    model->clr_group(_sc_group);
                                    }
                              }
                        else if (_sc_cmode) {
                              // Set, reset or toggle stop.
                              model->set_ifelm (_sc_group, v & 31, _sc_cmode - 1);
                              }
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const QList<MidiPatch*>& Aeolus::getPatchInfo() const
      {
      return patchList;
      }

//---------------------------------------------------------
//   effectParameter
//---------------------------------------------------------

SyntiParameter Aeolus::parameter(int id) const
      {
      SParmId spid(id);
      if (spid.syntiId != AEOLUS_ID)
            return SyntiParameter();

      switch (spid.subsystemId) {
            case 0:     // audio
                  return _audiopar[spid.paramId];
            case 1:
            case 2:
            case 3:
            case 4:
                  {
                  SyntiParameter* fp = _asectp[spid.subsystemId-1]->get_apar();
                  return fp[spid.paramId];
                  }

            default:
                  break;
            }
      return SyntiParameter();
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void Aeolus::setParameter(int id, double value)
      {
      SParmId spid(id);
      if (spid.syntiId != AEOLUS_ID)
            return;

      SyntiParameter* p = 0;

      switch(spid.subsystemId) {
            case 0:     // audio
                  p = &_audiopar[spid.paramId];
                  break;
            case 1:
            case 2:
            case 3:
            case 4:
                  {
                  SyntiParameter* fp = _asectp[spid.subsystemId-1]->get_apar();
                  p = &fp[spid.paramId];
                  }
            default:
                  break;
            }
      if (p == 0)
            return;
      if (value > p->max())
            value = p->max();
      else if (value < p->min())
            value = p->min();
// printf("aeolus set %d %d %f\n", effect, parameter, value);
      p->set(value);
      }

//---------------------------------------------------------
//   rewrite_label
//---------------------------------------------------------

void Aeolus::rewrite_label (const char *p)
      {
      strcpy (_tempstr, p);
      char* t = strstr (_tempstr, "-$");
      if (t)
            strcpy (t, t + 2);
      else {
            t = strchr (_tempstr, '$');
            if (t)
                  *t = ' ';
            }
      }

//---------------------------------------------------------
//   printGui
//---------------------------------------------------------

void Aeolus::printGui()
      {
      for (int i = 0; i < _ifc_init->_ndivis; ++i) {
            int group = i;
            rewrite_label (_ifc_init->_groupd [group]._label);
            printf ("Stops in group %s\n", _tempstr);
            uint32_t m = _ifelms [group];
            int n = _ifc_init->_groupd [group]._nifelm;
            for (int i = 0; i < n; i++) {
                  rewrite_label (_ifc_init->_groupd [group]._ifelmd [i]._label);
                  printf ("  %c %-7s %-1s\n", (m & 1) ? '+' : '-', _ifc_init->_groupd [group]._ifelmd [i]._mnemo, _tempstr);
                  m >>= 1;
                  }
            }
      }

SyntiState Aeolus::state() const
      {
      return SyntiState();
      }

void Aeolus::setState(const SyntiState&)
      {
      }

void Aeolus::allSoundsOff(int)
      {
      memset(_keymap, 0, sizeof(_keymap));
      }

void Aeolus::allNotesOff(int)
      {
      memset(_keymap, 0, sizeof(_keymap));
      }


