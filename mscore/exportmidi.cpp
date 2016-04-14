//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "exportmidi.h"
#include "midi/midifile.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/tempo.h"
#include "synthesizer/event.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader()
      {
      if (mf.tracks().isEmpty())
            return;
      MidiTrack &track  = mf.tracks().front();
#if 0 // TODO
      MeasureBase* measure  = cs->first();

      foreach (const Element* e, *measure->el()) {
            if (e->type() == Element::TEXT) {
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
                              track.insert(ev);
                              break;
                        case TEXT_SUBTITLE:
                              ev.setMetaType(META_SUBTITLE);
                              track.insert(ev);
                              break;
                        case TEXT_COMPOSER:
                              ev.setMetaType(META_COMPOSER);
                              track.insert(ev);
                              break;
                        case TEXT_TRANSLATOR:
                              ev.setMetaType(META_TRANSLATOR);
                              track.insert(ev);
                              break;
                        case TEXT_POET:
                              ev.setMetaType(META_POET);
                              track.insert(ev);
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

            auto bs = sigmap->lower_bound(startTick);
            auto es = sigmap->lower_bound(endTick);

            for (auto is = bs; is != es; ++is) {
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
                              qDebug("ExportMidi: unknown time signature %s",
                                 qPrintable(ts.print()));
                              break;
                        }
                  data[1] = n;
                  data[2] = 24;
                  data[3] = 8;

                  MidiEvent ev;
                  ev.setType(ME_META);
                  ev.setMetaType(META_TIME_SIGNATURE);
                  ev.setEData(data);
                  ev.setLen(4);
                  track.insert(is->first + tickOffset, ev);
                  }
            }

      //---------------------------------------------------
      //    write key signatures
      //    assume every staff corresponds to a midi track
      //---------------------------------------------------

      int staffIdx = 0;
      for (auto &track: mf.tracks()) {
            Staff* staff  = cs->staff(staffIdx);
            KeyList* keys = staff->keyList();

            foreach(const RepeatSegment* rs, *cs->repeatList()) {
                  int startTick  = rs->tick;
                  int endTick    = startTick + rs->len;
                  int tickOffset = rs->utick - rs->tick;

                  auto sk = keys->lower_bound(startTick);
                  auto ek = keys->lower_bound(endTick);

                  bool keysigFound = false;
                  for (auto ik = sk; ik != ek; ++ik) {
                        keysigFound = true;
                        MidiEvent ev;
                        ev.setType(ME_META);
                        Key key       = ik->second.key();   // -7 -- +7
                        ev.setMetaType(META_KEY_SIGNATURE);
                        ev.setLen(2);
                        unsigned char* data = new unsigned char[2];
                        data[0]   = int(key);
                        data[1]   = 0;  // major
                        ev.setEData(data);
                        track.insert(ik->first + tickOffset, ev);
                        }
                  if (!keysigFound) {
                        MidiEvent ev;
                        ev.setType(ME_META);
                        int key = 0;
                        ev.setMetaType(META_KEY_SIGNATURE);
                        ev.setLen(2);
                        unsigned char* data = new unsigned char[2];
                        data[0]   = key;
                        data[1]   = 0;  // major
                        ev.setEData(data);
                        track.insert(0, ev);
                        }
                  }
            ++staffIdx;
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

            auto se = tempomap->lower_bound(startTick);
            auto ee = tempomap->lower_bound(endTick);
            for (auto it = se; it != ee; ++it) {
                  MidiEvent ev;
                  ev.setType(ME_META);
                  //
                  // compute midi tempo: microseconds / quarter note
                  //
                  int tempo = lrint((1.0 / (it->second.tempo * relTempo)) * 1000000.0);

                  ev.setMetaType(META_TEMPO);
                  ev.setLen(3);
                  unsigned char* data = new unsigned char[3];
                  data[0]   = tempo >> 16;
                  data[1]   = tempo >> 8;
                  data[2]   = tempo;
                  ev.setEData(data);
                  track.insert(it->first + tickOffset, ev);
                  }
            }
      }

//---------------------------------------------------------
//  write
//    export midi file
//    return false on error
//---------------------------------------------------------

bool ExportMidi::write(const QString& name, bool midiExpandRepeats)
      {
      f.setFileName(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      mf.setDivision(MScore::division);
      mf.setFormat(1);
      QList<MidiTrack>& tracks = mf.tracks();

      for (int i = 0; i < cs->nstaves(); ++i)
            tracks.append(MidiTrack());

      cs->updateSwing();
      cs->createPlayEvents();
      cs->updateRepeatList(midiExpandRepeats);
      writeHeader();

      int staffIdx = 0;
      for (auto &track: tracks) {
            Staff* staff = cs->staff(staffIdx);
            Part* part   = staff->part();

            track.setOutPort(part->midiPort());
            track.setOutChannel(part->midiChannel());

            // Render each staff only once
            EventMap events;
            cs->renderStaff(&events, staff);
            cs->renderSpanners(&events, staffIdx);

            // Pass throught the all instruments in the part
            const InstrumentList* il = part->instruments();
            for(auto j = il->begin(); j!= il->end(); j++) {
                  // Pass throught the all channels of the instrument
                  // "normal", "pizzicato", "tremolo" for Strings,
                  // "normal", "mute" for Trumpet
                  foreach(const Channel* ch, j->second->channel()) {
                        char port    = part->masterScore()->midiPort(ch->channel);
                        char channel = part->masterScore()->midiChannel(ch->channel);

                        if (staff->isTop()) {
                              track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_RESET_ALL_CTRL, 0));
                              // We need this to get the correct pitch of bends
                              // Hidden under preferences because some software
                              // crashes when receiving RPNs: https://musescore.org/en/node/37431
                              if (channel != 9 && preferences.midiExportRPNs) {
                                    // set pitch bend sensitivity to 12 semitones:
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 0));
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 0));
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HDATA, 12));

                                    // reset fine tuning
                                    /*track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 1));
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 0));
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HDATA, 64));*/

                                    // deactivate rpn
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 127));
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 127));
                              }

                              if (ch->program != -1)
                                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_PROGRAM, ch->program));
                              track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_VOLUME, ch->volume));
                              track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_PANPOT, ch->pan));
                              track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_REVERB_SEND, ch->reverb));
                              track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_CHORUS_SEND, ch->chorus));
                              }

                        // Export port to MIDI META event
                        if (track.outPort() >= 0 && track.outPort() <= 127) {
                              MidiEvent ev;
                              ev.setType(ME_META);
                              ev.setMetaType(META_PORT_CHANGE);
                              ev.setLen(1);
                              unsigned char* data = new unsigned char[1];
                              data[0] = int(track.outPort());
                              ev.setEData(data);
                              track.insert(0, ev);
                              }

                        for (auto i = events.begin(); i != events.end(); ++i) {
                              NPlayEvent event(i->second);
                              char eventPort    = cs->masterScore()->midiPort(event.channel());
                              char eventChannel = cs->masterScore()->midiChannel(event.channel());
                              if (port != eventPort || channel != eventChannel)
                                    continue;

                              if (event.type() == ME_NOTEON) {
                                    track.insert(i->first, MidiEvent(ME_NOTEON, channel,
                                                                     event.pitch(), event.velo()));
                                    }
                              else if (event.type() == ME_CONTROLLER) {
                                    track.insert(i->first, MidiEvent(ME_CONTROLLER, channel,
                                                                     event.controller(), event.value()));
                                    }
                              else if(event.type() == ME_PITCHBEND) {
                                    track.insert(i->first, MidiEvent(ME_PITCHBEND, channel,
                                                                     event.dataA(), event.dataB()));
                                    }
                              else {
                                    qDebug("writeMidi: unknown midi event 0x%02x", event.type());
                                    }
                              }
                        }
                  }
            ++staffIdx;
            }
      return !mf.write(&f);
      }
}

