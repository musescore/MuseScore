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

#ifdef Q_OS_MAC
#include <locale.h>
#endif

#include "aeolus.h"
#include "model.h"

namespace Ms {
      extern QString dataPath;
      extern QString mscoreGlobalShare;
      };

#include "synthesizer/event.h"
#include "libmscore/xml.h"
#include "sparm_p.h"

const std::vector<ParDescr> Aeolus::pd = {
      { A_VOLUME,  "volume",  true, 0.32f,   0.00f,  1.00f },
      { A_REVSIZE, "revsize", true, 0.075f,  0.025f, 0.15f },
      { A_REVTIME, "revtime", true, 4.0f,    2.0f,   7.00f },
      { A_STPOSIT, "stposit", true, 0.5f,   -1.00f,  1.0f  },
      };

//---------------------------------------------------------
//   createAeolus
//---------------------------------------------------------

Synthesizer* createAeolus()
      {
      return new Aeolus();
      }

//---------------------------------------------------------
//   Aeolus
//---------------------------------------------------------

Aeolus::Aeolus()
      {
      model = 0;
      patchList.append(new MidiPatch { false, "Aeolus", 0, 0, 0, "Aeolus" });

      _sc_cmode = 0;
      _sc_group = 0;
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
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Aeolus::init(float samplerate)
      {
      setlocale(LC_ALL, "C"); // scanf of floats does not work otherwise

      QString stops = mscoreGlobalShare + "/sound/aeolus/stops";
      int n = strlen(qPrintable(stops));
      char* stopsPath = new char[n+1];
      strcpy(stopsPath, qPrintable(stops));

      QDir dir;
      QString waves = dataPath + QString("/aeolus/waves%1").arg(int(samplerate));
      dir.mkpath(waves);
      n = strlen(qPrintable(waves));
      char* wavesPath = new char[n+1];
      strcpy(wavesPath, qPrintable(waves));

      audio_init(int(samplerate));
      model = new Model (this, _midimap, stopsPath, "Aeolus", wavesPath);

      audio_start();
      model->init();
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

void Aeolus::play(const PlayEvent& event)
      {
      int ch   = event.channel();
      int type = event.type();
      int m    = _midimap [ch] & 0x7f;        // Keyboard and hold bits

      if (type == ME_NOTEON) {
            int n = event.dataA();
            int v = event.dataB();
            if ((n >= 36) && (n <= 96)) {
                  n -= 36;
                  if (v == 0)
                        key_off(n, m);
                  else
                        key_on(n, m);
                  }
            }
      else if (type == ME_NOTEOFF) {
            int n = event.dataA();
            if ((n >= 36) && (n <= 96)) {
                  n -= 36;
                  key_off(n, m);
                  }
            }
      else if (type == ME_CONTROLLER) {
            int p = event.dataA();
            int v = event.dataB();
            switch(p) {
                  case MIDICTL_IFELM:
                        if (v & 0x40) {
                              // Set mode or clear group.
                              _sc_cmode = (v >> 4) & 3;
                              _sc_group = v & 7;
                              if (_sc_cmode == 0)
                                    model->clr_group(_sc_group);
                              }
                        else if (_sc_cmode) {
                              // Set, reset or toggle stop.
                              model->set_ifelm (_sc_group, v & 0x1f, _sc_cmode - 1);
                              }
                        break;
                  case CTRL_ALL_NOTES_OFF:
                        allNotesOff(ch);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void Aeolus::allNotesOff(int)
      {
      for (int i = 0; i < NNOTES; ++i)
            _keymap[i] = 0x80;
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const QList<MidiPatch*>& Aeolus::getPatchInfo() const
      {
      return patchList;
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void Aeolus::setValue(int id, double value)
      {
      const ParDescr* p = parameter(id);
      if (p == 0)
            return;
      double v;
      if (p->log)
            v = exp(p->min + value * (p->max - p->min));
      else
            v = p->min + value * (p->max - p->min);

      switch (id) {
            case A_VOLUME:   _audiopar[VOLUME]  = v; break;
            case A_REVSIZE:  _audiopar[REVSIZE] = v; break;
            case A_REVTIME:  _audiopar[REVTIME] = v; break;
            case A_STPOSIT:  _audiopar[STPOSIT] = v; break;
            }
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

double Aeolus::value(int idx) const
      {
      double v = 0.0;
      switch (idx) {
            case A_VOLUME:   v = _audiopar[VOLUME];  break;
            case A_REVSIZE:  v = _audiopar[REVSIZE]; break;
            case A_REVTIME:  v = _audiopar[REVTIME]; break;
            case A_STPOSIT:  v = _audiopar[STPOSIT]; break;
            }
      const ParDescr* p = parameter(idx);
      if (p->log)
            v = (log(v) - p->min)/(p->max - p->min);
      else
            v = (v - p->min)/(p->max - p->min);
      return v;
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerGroup Aeolus::state() const
      {
      SynthesizerGroup g;
      g.setName(name());

      for (const ParDescr& d : pd)
            g.push_back(IdValue(d.id, QString("%1").arg(value(d.id))));
      return g;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Aeolus::setState(const SynthesizerGroup& g)
      {
      for (const IdValue& v : g)
            setValue(v.id, v.data.toDouble());
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

const ParDescr* Aeolus::parameter(int idx) const
      {
      return &pd[idx];
      }

