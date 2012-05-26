//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: musescore.cpp 4961 2011-11-11 16:24:17Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include <fenv.h>

#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/instrument.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/undo.h"
#include "mixer.h"
#include "scoreview.h"
#include "playpanel.h"
#include "seq.h"
#include "preferences.h"

#ifdef OSC
#include "ofqf/qoscserver.h"
static int oscPort = 5282;
#endif

//---------------------------------------------------------
//   initOsc
//---------------------------------------------------------

#ifndef OSC
void MuseScore::initOsc()
      {
      }

#else // #ifndef OSC

//---------------------------------------------------------
//   initOsc
//---------------------------------------------------------

void MuseScore::initOsc()
      {
      if (!preferences.useOsc)
            return;
      int port;
      if (oscPort)
            port = oscPort;
      else
            port = preferences.oscPort;
      QOscServer* osc = new QOscServer(port, qApp);
      PathObject* oo = new PathObject( "/mscore", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscIntMessage(int)));
      oo = new PathObject( "/play",QVariant::Invalid, osc);
      QObject::connect(oo, SIGNAL(data()), SLOT(oscPlay()));
      oo = new PathObject( "/stop", QVariant::Invalid, osc);
      QObject::connect(oo, SIGNAL(data()), SLOT(oscStop()));
      oo = new PathObject( "/tempo", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscTempo(int)));
      oo = new PathObject( "/volume", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscVolume(int)));
      oo = new PathObject( "/next", QVariant::Invalid, osc);
      QObject::connect(oo, SIGNAL(data()), SLOT(oscNext()));
      oo = new PathObject( "/next-measure", QVariant::Invalid, osc);
      QObject::connect(oo, SIGNAL(data()), SLOT(oscNextMeasure()));
      oo = new PathObject( "/goto", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscGoto(int)));
      oo = new PathObject( "/select-measure", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscSelectMeasure(int)));
      for(int i=1; i <=12; i++ ) {
            oo = new PathObject( QString("/vol%1").arg(i), QVariant::Double, osc);
            QObject::connect(oo, SIGNAL(data(double)), SLOT(oscVolChannel(double)));
            }
      for(int i=1; i <=12; i++ ) {
            oo = new PathObject( QString("/pan%1").arg(i), QVariant::Double, osc);
            QObject::connect(oo, SIGNAL(data(double)), SLOT(oscPanChannel(double)));
            }
      for(int i=1; i <=12; i++ ) {
            oo = new PathObject( QString("/mute%1").arg(i), QVariant::Double, osc);
            QObject::connect(oo, SIGNAL(data(double)), SLOT(oscMuteChannel(double)));
            }
      oo = new PathObject( "/open", QVariant::String, osc);
      QObject::connect(oo, SIGNAL(data(QString)), SLOT(oscOpen(QString)));
      oo = new PathObject( "/close-all", QVariant::Invalid, osc);
      QObject::connect(oo, SIGNAL(data()), SLOT(oscCloseAll()));
      oo = new PathObject( "/plugin", QVariant::String, osc);
      QObject::connect(oo, SIGNAL(data(QString)), SLOT(oscTriggerPlugin(QString)));

      oo = new PathObject( "/color-note", QVariant::List, osc);
      QObject::connect(oo, SIGNAL(data(QVariantList)), SLOT(oscColorNote(QVariantList)));

      }

//---------------------------------------------------------
//   oscIntMessage
//---------------------------------------------------------

void MuseScore::oscIntMessage(int val)
      {
      if (val < 128)
            midiNoteReceived(0, val, false);
      else
            midiCtrlReceived(val-128, 22);
      }

void MuseScore::oscPlay()
      {
      QAction* a = getAction("play");
      if (!a->isChecked())
            a->trigger();
      }

void MuseScore::oscStop()
      {
      QAction* a = getAction("play");
      if (a->isChecked())
            a->trigger();
      }

void MuseScore::oscNext()
      {
      qDebug("next");
      QAction* a = getAction("next-chord");
      a->trigger();
      }

void MuseScore::oscNextMeasure()
      {
      QAction* a = getAction("next-measure");
      a->trigger();
      }

void MuseScore::oscGoto(int m)
      {
      qDebug("GOTO %d", m);
      if (cv == 0)
            return;
      cv->search(m);
      }

void MuseScore::oscSelectMeasure(int m)
      {
      qDebug("SelectMeasure %d", m);
      if (cv == 0)
            return;
      cv->selectMeasure(m);
      }


void MuseScore::oscOpen(QString path)
      {
      qDebug("Open %s", qPrintable(path));
      openScore(path);
      }


void MuseScore::oscCloseAll()
      {
      qDebug("CloseAll");
      while(cs != 0)
          closeScore(cs);
      }

//---------------------------------------------------------
//   oscTempo
//---------------------------------------------------------

void MuseScore::oscTempo(int val)
      {
      if (val < 0)
            val = 0;
      if (val > 127)
            val = 127;
      val = (val * 240) / 128;
      if (playPanel)
            playPanel->setRelTempo(val);
      if (seq)
            seq->setRelTempo(double(val));
      }

//---------------------------------------------------------
//   oscTriggerPlugin
//---------------------------------------------------------
void MuseScore::oscTriggerPlugin(QString s)
      {
      QStringList args = s.split(",");
      if(args.length() > 0) {
            int idx = pluginIdxFromPath(args.at(0));
            if(idx != -1) {
                  for(int i = 1; i < args.length()-1; i++) {
                        addGlobalObjectToPluginEngine(qPrintable(args.at(i)), args.at(i+1));
                        i++;
                        }
                  pluginTriggered(idx);
                  }
            }
      }

//---------------------------------------------------------
//   oscColorNote
//---------------------------------------------------------
void MuseScore::oscColorNote(QVariantList list)
      {
      qDebug() << list;
      if(!cs)
            return;
      if (list.length() != 2 && list.length() != 3)
            return;
      int tick;
      int pitch;
      QColor noteColor("red"); //default to red
      bool ok;
      tick = list[0].toInt(&ok);
      if (!ok)
            return;
      pitch = list[1].toInt(&ok);
      if (!ok)
            return;
      if(list.length() == 3 && list[2].canConvert(QVariant::String)) {
            QColor color(list[2].toString());
            if(color.isValid())
                  noteColor = color;
            }
      
      Measure* measure = cs->tick2measure(tick);
      if(!measure)
            return;
      Segment* s = measure->findSegment(SegChordRest, tick);
      if (!s)
            return;
      //get all chords in segment...
      int n = cs->nstaves() * VOICES;
      for (int i = 0; i < n; i++) {
            Element* e = s->element(i);
            if (e && e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if(cr->type() == CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        for (int idx = 0; idx < chord->notes().length(); idx++) {
                              Note* note = chord->notes()[idx];
                              if (note->pitch() == pitch) {
                                    cs->startCmd();
                                    cs->undo(new ChangeProperty(note, P_COLOR, noteColor));
                                    cs->endCmd();
                                    cs->end();
                                    return;
                                    }
                              }
                        }
                  }
            }
      }
//---------------------------------------------------------
//   oscVolume
//---------------------------------------------------------

void MuseScore::oscVolume(int val)
      {
      double v = val / 128.0;
      if (seq)
            seq->setGain(v);
      }

//---------------------------------------------------------
//   oscVolChannel
//---------------------------------------------------------

void MuseScore::oscVolChannel(double val)
      {
      if(!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(4).toInt() - 1;
      QList<MidiMapping>* mms = cs->midiMapping();
      if( i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            int iv = lrint(val*127);
            seq->setController(channel->channel, CTRL_VOLUME, iv);
            channel->volume = iv;
            if (iledit)
                  iledit->partEdit(i)->volume->setValue(iv);
            }
      }

//---------------------------------------------------------
//   oscPanChannel
//---------------------------------------------------------

void MuseScore::oscPanChannel(double val)
      {
      if(!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(4).toInt() - 1;
      QList<MidiMapping>* mms = cs->midiMapping();
      if( i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            int iv = lrint((val + 1) * 64);
            seq->setController(channel->channel, CTRL_PANPOT, iv);
            channel->volume = iv;
            if (iledit)
                  iledit->partEdit(i)->pan->setValue(iv);
            }
      }

//---------------------------------------------------------
//   oscMuteChannel
//---------------------------------------------------------

void MuseScore::oscMuteChannel(double val)
      {
      if(!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(5).toInt() - 1;
      QList<MidiMapping>* mms = cs->midiMapping();
      if( i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            channel->mute = (val==0.0f ? false : true);
            if (iledit)
                  iledit->partEdit(i)->mute->setCheckState(val==0.0f ? Qt::Unchecked:Qt::Checked);
            }
      }
#endif // #ifndef OSC
