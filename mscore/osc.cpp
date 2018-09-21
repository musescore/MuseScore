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
#include "parteditbase.h"
#include "scoreview.h"
#include "playpanel.h"
#include "preferences.h"
#include "seq.h"
#include "synthesizer/msynthesizer.h"
#include "shortcut.h"

#ifdef OSC
#include "ofqf/qoscserver.h"
#endif

namespace Ms {

extern MasterSynthesizer* synti;

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
      if (!preferences.getBool(PREF_IO_OSC_USEREMOTECONTROL))
            return;
      int port = preferences.getInt(PREF_IO_OSC_PORTNUMBER);
      QOscServer* osc = new QOscServer(port, qApp);

      PathObject* oo = new PathObject( "/addpitch", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscIntMessage(int)));

      oo = new PathObject( "/tempo", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscTempo(int)));
      oo = new PathObject( "/volume", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscVolume(int)));
      oo = new PathObject( "/goto", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscGoto(int)));
      oo = new PathObject( "/select-measure", QVariant::Int, osc);
      QObject::connect(oo, SIGNAL(data(int)), SLOT(oscSelectMeasure(int)));
      for (int i = 1; i <= 12; i++ ) {
            oo = new PathObject( QString("/vol%1").arg(i), QVariant::Double, osc);
            QObject::connect(oo, SIGNAL(data(double)), SLOT(oscVolChannel(double)));
            }
      for(int i = 1; i <= 12; i++ ) {
            oo = new PathObject( QString("/pan%1").arg(i), QVariant::Double, osc);
            QObject::connect(oo, SIGNAL(data(double)), SLOT(oscPanChannel(double)));
            }
      for(int i = 1; i <= 12; i++ ) {
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

      for (const Shortcut* s : Shortcut::shortcuts()) {
            oo = new PathObject( QString("/actions/%1").arg(s->key().data()), QVariant::Invalid, osc);
            QObject::connect(oo, SIGNAL(data()), SLOT(oscAction()));
            }
      }

//---------------------------------------------------------
//   oscIntMessage
//---------------------------------------------------------

void MuseScore::oscIntMessage(int val)
      {
      if (val < 128) {
            midiNoteReceived(0, val, 60);
            midiNoteReceived(0, val, 0);
            }
      else
            midiCtrlReceived(val-128, 22);
      }

void MuseScore::oscAction()
      {
      PathObject* pathObject = qobject_cast<PathObject*>(sender());
      QString path = pathObject->path().mid(9);
      QAction* a = getAction(path.toLocal8Bit().data());
      a->trigger();
      }

void MuseScore::oscGoto(int m)
      {
      qDebug("GOTO %d", m);
      if (cv == 0)
            return;
      cv->searchPage(m);
      }

void MuseScore::oscSelectMeasure(int m)
      {
      qDebug("SelectMeasure %d", m);
      if (cv == 0)
            return;
//      cv->selectMeasure(m);
      Score* score = cv->score();
      int i = 0;
      for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (++i < m)
                  continue;
            score->selection().setState(SelState::RANGE);
            score->selection().setStartSegment(measure->first());
            score->selection().setEndSegment(measure->last());
            score->selection().setStaffStart(0);
            score->selection().setStaffEnd(score->nstaves());
            score->selection().updateSelectedElements();
            score->selection().setState(SelState::RANGE);
            score->addRefresh(measure->canvasBoundingRect());
            cv->adjustCanvasPosition(measure, true);
            score->setUpdateAll();
            score->update();
            break;
            }
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
      if (val < 10)
            val = 10;
      if (val > 300)
            val = 300;
      qreal t = val * .01;
      if (playPanel)
            playPanel->setRelTempo(t);
      if (seq)
            seq->setRelTempo(double(t));
      }

//---------------------------------------------------------
//   oscTriggerPlugin
//---------------------------------------------------------

void MuseScore::oscTriggerPlugin(QString /*s*/)
      {
#if 0 // TODO
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
#endif
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
      Segment* s = measure->findSegment(SegmentType::ChordRest, tick);
      if (!s)
            return;
      //get all chords in segment...
      int n = cs->nstaves() * VOICES;
      for (int i = 0; i < n; i++) {
            Element* e = s->element(i);
            if (e && e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if (cr->type() == ElementType::CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        for (Note* note : chord->notes()) {
                              if (note->pitch() == pitch) {
                                    cs->startCmd();
                                    cs->undo(new ChangeProperty(note, Pid::COLOR, noteColor));
                                    cs->endCmd();
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
      synti->setGain(v);
      }

//---------------------------------------------------------
//   oscVolChannel
//---------------------------------------------------------

void MuseScore::oscVolChannel(double val)
      {
      if (!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(4).toInt() - 1;
      QList<MidiMapping>* mms = cs->masterScore()->midiMapping();
      if( i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            int iv = lrint(val*127);
            seq->setController(channel->channel(), CTRL_VOLUME, iv);
            channel->setVolume(val * 100.0);
            if (mixer)
                  mixer->getPartAtIndex(i)->volume->setValue(val * 100.0);
            }
      }

//---------------------------------------------------------
//   oscPanChannel
//---------------------------------------------------------

void MuseScore::oscPanChannel(double val)
      {
      if (!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(4).toInt() - 1;
      QList<MidiMapping>* mms = cs->masterScore()->midiMapping();
      if (i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            int iv = lrint((val + 1) * 64);
            seq->setController(channel->channel(), CTRL_PANPOT, iv);
            channel->setPan(val * 180.0);
            if (mixer)
                  mixer->getPartAtIndex(i)->pan->setValue(val * 100.0);
            }
      }

//---------------------------------------------------------
//   oscMuteChannel
//---------------------------------------------------------

void MuseScore::oscMuteChannel(double val)
      {
      if (!cs)
            return;
      PathObject* po = (PathObject*) sender();

      int i = po->path().mid(5).toInt() - 1;
      QList<MidiMapping>* mms = cs->masterScore()->midiMapping();
      if (i >= 0 && i < mms->size()) {
            MidiMapping mm = mms->at(i);
            Channel* channel = mm.articulation;
            channel->setMute(val==0.0f ? false : true);
            if (mixer)
                  mixer->getPartAtIndex(i)->mute->setChecked(channel->mute());
            }
      }
#endif // #ifndef OSC
}

