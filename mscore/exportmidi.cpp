//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: exportmidi.cpp 5350 2012-02-20 22:01:29Z lasconic $
//
//  Copyright (C) 2002-20011 Werner Schweer and others
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

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/tempo.h"
#include "midifile.h"
#include "libmscore/event.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "preferences.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"

//---------------------------------------------------------
//   exportMidi
//---------------------------------------------------------

class ExportMidi {
      QFile f;
      Score* cs;

      void writeHeader();

   public:
      MidiFile mf;

      ExportMidi(Score* s) { cs = s; }
      bool write(const QString& name);
      };

//---------------------------------------------------------
//   exportMidi
//    return false on error
//---------------------------------------------------------

bool saveMidi(Score* score, const QString& name)
      {
      ExportMidi em(score);
      return em.write(name);
      }

//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader()
      {
      if (mf.tracks()->isEmpty())
            return;
      MidiTrack* track  = mf.tracks()->front();
#if 0 // TODOxx
      MeasureBase* measure  = cs->first();

      foreach (const Element* e, *measure->el()) {
            if (e->type() == TEXT) {
                  const Text* text = (const Text*)(e);
                  QString str = text->getText();
                  int len     = str.length() + 1;
                  unsigned char* data = new unsigned char[len];
                  strcpy((char*)(data), str.toLatin1().data());
                  Event ev(ME_META);
                  ev.setOntime(0);
                  ev.setData(data);
                  ev.setLen(len);

                  switch (text->subtype()) {
                        case TEXT_TITLE:
                              ev.setMetaType(META_TITLE);
                              track->insert(ev);
                              break;
                        case TEXT_SUBTITLE:
                              ev.setMetaType(META_SUBTITLE);
                              track->insert(ev);
                              break;
                        case TEXT_COMPOSER:
                              ev.setMetaType(META_COMPOSER);
                              track->insert(ev);
                              break;
                        case TEXT_TRANSLATOR:
                              ev.setMetaType(META_TRANSLATOR);
                              track->insert(ev);
                              break;
                        case TEXT_POET:
                              ev.setMetaType(META_POET);
                              track->insert(ev);
                              break;
                        default:
                              break;
                        }
                  }
            }
#endif
      //--------------------------------------------
      //    write time signature
      //--------------------------------------------

      TimeSigMap* sigmap = cs->sigmap();
      foreach(const RepeatSegment* rs, *cs->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;

            iSigEvent bs = sigmap->lower_bound(startTick);
            iSigEvent es = sigmap->lower_bound(endTick);

            for (iSigEvent is = bs; is != es; ++is) {
                  SigEvent se   = is->second;
                  unsigned char* data = new unsigned char[4];
                  Fraction ts(se.timesig());
                  data[0] = ts.numerator();
                  int n;
                  switch (ts.denominator()) {
                        case 1:  n = 0; break;
                        case 2:  n = 1; break;
                        case 4:  n = 2; break;
                        case 8:  n = 3; break;
                        case 16: n = 4; break;
                        case 32: n = 5; break;
                        default:
                              n = 2;
                              qDebug("ExportMidi: unknown time signature %s\n",
                                 qPrintable(ts.print()));
                              break;
                        }
                  data[1] = n;
                  data[2] = 24;
                  data[3] = 8;

                  Event ev(ME_META);
                  ev.setMetaType(META_TIME_SIGNATURE);
                  ev.setData(data);
                  ev.setLen(4);
                  ev.setOntime(is->first + tickOffset);
                  track->insert(ev);
                  }
            }

      //---------------------------------------------------
      //    write key signatures
      //    assume every staff corresponds to a midi track
      //---------------------------------------------------

      foreach(MidiTrack* track, *mf.tracks()) {
            Staff* staff      = track->staff();
            KeyList* keymap   = staff->keymap();

            foreach(const RepeatSegment* rs, *cs->repeatList()) {
                  int startTick  = rs->tick;
                  int endTick    = startTick + rs->len;
                  int tickOffset = rs->utick - rs->tick;

                  iKeyList sk = keymap->lower_bound(startTick);
                  iKeyList ek = keymap->lower_bound(endTick);

                  bool keysigFound = false;
                  for (iKeyList ik = sk; ik != ek; ++ik) {
                        keysigFound = true;
                        Event ev(ME_META);
                        ev.setOntime(ik->first + tickOffset);
                        int key       = ik->second.accidentalType();   // -7 -- +7
                        ev.setMetaType(META_KEY_SIGNATURE);
                        ev.setLen(2);
                        unsigned char* data = new unsigned char[2];
                        data[0]   = key;
                        data[1]   = 0;  // major
                        ev.setData(data);
                        track->insert(ev);
                        }
                  if (!keysigFound) {
                        Event ev(ME_META);
                        ev.setOntime(0);
                        int key = 0;
                        ev.setMetaType(META_KEY_SIGNATURE);
                        ev.setLen(2);
                        unsigned char* data = new unsigned char[2];
                        data[0]   = key;
                        data[1]   = 0;  // major
                        ev.setData(data);
                        track->insert(ev);
                        }
                  }
            }

      //--------------------------------------------
      //    write tempo changes
      //--------------------------------------------

      TempoMap* tempomap = cs->tempomap();
      int relTempo = tempomap->relTempo();      
      foreach(const RepeatSegment* rs, *cs->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;

            iTEvent se = tempomap->lower_bound(startTick);
            iTEvent ee = tempomap->lower_bound(endTick);
            for (iTEvent it = se; it != ee; ++it) {
                  Event ev(ME_META);
                  ev.setOntime(it->first + tickOffset);
                  //
                  // compute midi tempo: microseconds / quarter note
                  //
                  int tempo = lrint((1.0 / (it->second.tempo * relTempo * 0.01)) * 1000000.0);

                  ev.setMetaType(META_TEMPO);
                  ev.setLen(3);
                  unsigned char* data = new unsigned char[3];
                  data[0]   = tempo >> 16;
                  data[1]   = tempo >> 8;
                  data[2]   = tempo;
                  ev.setData(data);
                  track->insert(ev);
                  }
            }
      }

//---------------------------------------------------------
//  write
//    export midi file
//    return false on error
//---------------------------------------------------------

bool ExportMidi::write(const QString& name)
      {
      f.setFileName(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      mf.setDivision(MScore::division);
      mf.setFormat(1);
      QList<MidiTrack*>* tracks = mf.tracks();

      foreach(Staff* staff, cs->staves()) {
            // if (!staff->primaryStaff())
            // continue;
            MidiTrack* t= new MidiTrack(&mf);
            t->setStaff(staff);
            tracks->append(t);
            }

      cs->updateRepeatList(preferences.midiExpandRepeats);
      writeHeader();

      foreach (MidiTrack* track, *tracks) {
            Staff* staff = track->staff();
            Part* part   = staff->part();
            int channel  = part->midiChannel();
            track->setOutPort(0);
            track->setOutChannel(channel);

            if (staff->isTop()) {
                  // set pitch bend sensitivity to 12 semitones:
                  track->addCtrl(0, channel, CTRL_LRPN, 0);
                  track->addCtrl(0, channel, CTRL_HRPN, 0);
                  track->addCtrl(0, channel, CTRL_HDATA, 12);

                  // reset fine tuning
                  track->addCtrl(0, channel, CTRL_LRPN, 1);
                  track->addCtrl(0, channel, CTRL_HRPN, 0);
                  track->addCtrl(0, channel, CTRL_HDATA, 64);

                  // deactivate rpn
                  track->addCtrl(0, channel, CTRL_LRPN, 127);
                  track->addCtrl(0, channel, CTRL_HRPN, 127);

                  if (part->midiProgram() != -1)
                        track->addCtrl(0, channel, CTRL_PROGRAM, part->midiProgram());
                  track->addCtrl(0, channel, CTRL_VOLUME, part->volume());
                  track->addCtrl(0, channel, CTRL_PANPOT, part->pan());
                  track->addCtrl(0, channel, CTRL_REVERB_SEND, part->reverb());
                  track->addCtrl(0, channel, CTRL_CHORUS_SEND, part->chorus());
                  }


            EventMap events;
            cs->renderPart(&events, part);

            for (EventMap::const_iterator i = events.begin(); i != events.end(); ++i) {
                  Event event = i.value();
                  if (event.channel() != channel)
                        continue;
                  if (event.type() == ME_NOTEON) {
                        Event ne(ME_NOTEON);
                        ne.setOntime(i.key());
                        ne.setChannel(event.channel());
                        ne.setPitch(event.pitch());
                        ne.setVelo(event.velo());
                        track->insert(ne);
                        }
                  else if (event.type() == ME_CONTROLLER) {
                        track->addCtrl(i.key(), event.channel(), event.controller(), event.value());
                        }
                  else {
                        qDebug("writeMidi: unknown midi event 0x%02x\n", event.type());
                        }
                  }
            }
      return !mf.write(&f);
      }
