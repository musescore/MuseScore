//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: event.cpp 4926 2011-10-29 18:13:35Z wschweer $
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/xml.h"
#include "libmscore/note.h"
#include "libmscore/sig.h"
#include "event.h"

namespace Ms {

//---------------------------------------------------------
//   MidiCoreEvent::write
//---------------------------------------------------------

void MidiCoreEvent::write(Xml& xml) const
      {
      switch(_type) {
            case ME_NOTEON:
                  xml.tagE(QString("note-on  channel=\"%1\" pitch=\"%2\" velo=\"%3\"")
                     .arg(_channel).arg(_a).arg(_b));
                  break;

            case ME_NOTEOFF:
                  xml.tagE(QString("note-off  channel=\"%1\" pitch=\"%2\" velo=\"%3\"")
                     .arg(_channel).arg(_a).arg(_b));
                  break;

            case ME_CONTROLLER:
                  if (_a == CTRL_PROGRAM) {
                        if (_channel == 0) {
                              xml.tagE(QString("program value=\"%1\"").arg(_b));
                              }
                        else {
                              xml.tagE(QString("program channel=\"%1\" value=\"%2\"")
                                 .arg(channel()).arg(_b));
                              }
                        }
                  else {
                        if (channel() == 0) {
                              xml.tagE(QString("controller ctrl=\"%1\" value=\"%2\"")
                                 .arg(_a).arg(_b));
                              }
                        else {
                              xml.tagE(QString("controller channel=\"%1\" ctrl=\"%2\" value=\"%3\"")
                                 .arg(channel()).arg(_a).arg(_b));
                              }
                        }
                  break;
            default:
                  qDebug("MidiCoreEvent::write: unknown type");
                  break;
            }
      }

//---------------------------------------------------------
//   Event::Event
//---------------------------------------------------------

Event::Event()
      {
      _type            = 0;
      _ontime          = -1;
      _noquantOntime   = 0;
      _noquantDuration = 0;
      _channel         = 0;
      _a               = 0;
      _b               = 0;
      _duration        = 0;
      _tpc             = 0;
      _voice           = 0;
      _edata           = 0;
      _len             = 0;
      _metaType        = 0;
      _note            = 0;
      _tuning          = 0.0;
      }

Event::Event(int t)
      {
      _type            = t;
      _ontime          = -1;
      _noquantOntime   = 0;
      _noquantDuration = 0;
      _channel         = 0;
      _a               = 0;
      _b               = 0;
      _duration        = 0;
      _tpc             = 0;
      _voice           = 0;
      _edata            = 0;
      _len             = 0;
      _metaType        = 0;
      _note            = 0;
      _tuning          = 0.0;
      }

Event::Event(const Event& e)
   : PlayEvent(e)
      {
      _type       = e._type;
      _ontime     = e._ontime;
      _noquantOntime   = e._noquantOntime;
      _noquantDuration = e._noquantDuration;
      _channel    = e._channel;
      _a          = e._a;
      _b          = e._b;
      _duration   = e._duration;
      _tpc        = e._tpc;
      _voice      = e._voice;
      _notes      = e._notes;
      if (e._edata) {
            _edata = new unsigned char[e._len + 1];  // dont forget trailing zero
            memcpy(_edata, e._edata, e._len+1);
            }
      else
            _edata = 0;
      _len        = e._len;
      _metaType   = e._metaType;
      _note       = e._note;
      _tuning     = e._tuning;
      }

Event::~Event()
      {
      delete[] _edata;
      }

//---------------------------------------------------------
//   NPlayEvent::NPlayEvent (beatType2metronomeEvent)
//---------------------------------------------------------

NPlayEvent::NPlayEvent(BeatType beatType)
      {
      setType(ME_TICK2);
      setVelo(127);
      switch (beatType) {
            case BeatType::DOWNBEAT:
                  setType(ME_TICK1);
                  break;
            case BeatType::SIMPLE_STRESSED:
            case BeatType::COMPOUND_STRESSED:
                  // use defaults
                  break;
            case BeatType::SIMPLE_UNSTRESSED:
            case BeatType::COMPOUND_UNSTRESSED:
                  setVelo(80);
                  break;
            case BeatType::COMPOUND_SUBBEAT:
                  setVelo(25);
                  break;
            case BeatType::SUBBEAT:
                  setVelo(15);
                  break;
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Event::dump() const
      {
      printf("event ");
      switch (_type) {
            case ME_NOTEON:     printf("noteon    "); break;
            case ME_CONTROLLER: printf("controller"); break;
            case ME_PROGRAM:    printf("program   "); break;
            default:            printf("0x%02x    ", _type); break;
            }
      printf(" 0x%02x 0x%02x\n", _a, _b);
      }

//---------------------------------------------------------
//   isChannelEvent
//---------------------------------------------------------

bool MidiCoreEvent::isChannelEvent() const
      {
      switch(_type) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PROGRAM:
            case ME_AFTERTOUCH:
            case ME_PITCHBEND:
            case ME_NOTE:
            case ME_CHORD:
                  return true;
            default:
                  return false;
            }
      return false;
      }

//---------------------------------------------------------
//   Event::write
//---------------------------------------------------------

void Event::write(Xml& xml) const
      {
      switch(_type) {
            case ME_NOTE:
                  xml.tagE(QString("note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
                     .arg(_ontime).arg(_channel).arg(_duration).arg(_a).arg(_b));
                  break;

            case ME_NOTEON:
                  xml.tagE(QString("note-on  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(_ontime).arg(_channel).arg(_a).arg(_b));
                  break;

            case ME_NOTEOFF:
                  xml.tagE(QString("note-off  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(_ontime).arg(_channel).arg(_a).arg(_b));
                  break;

            case ME_CONTROLLER:
                  if (_a == CTRL_PROGRAM) {
                        if ((_ontime == -1) && (_channel == 0)) {
                              xml.tagE(QString("program value=\"%1\"").arg(_b));
                              }
                        else {
                              xml.tagE(QString("program tick=\"%1\" channel=\"%2\" value=\"%3\"")
                                 .arg(ontime()).arg(channel()).arg(_b));
                              }
                        }
                  else {
                        if ((ontime() == -1) && (channel() == 0)) {
                              xml.tagE(QString("controller ctrl=\"%1\" value=\"%2\"")
                                 .arg(_a).arg(_b));
                              }
                        else {
                              xml.tagE(QString("controller tick=\"%1\" channel=\"%2\" ctrl=\"%3\" value=\"%4\"")
                                 .arg(ontime()).arg(channel()).arg(_a).arg(_b));
                              }
                        }
                  break;

            case ME_SYSEX:
                  xml.stag(QString("sysex tick=\"%1\" len=\"%2\"").arg(ontime()).arg(_len));
                  xml.dump(_len, _edata);
                  xml.etag();
                  break;

            case ME_META:
                  switch(metaType()) {
                        case META_TRACK_NAME:
                              xml.tag(QString("TrackName tick=\"%1\"").arg(ontime()), QString((char*)(edata())));
                              break;

                        case META_LYRIC:
                              xml.tag(QString("Lyric tick=\"%1\"").arg(ontime()), QString((char*)(edata())));
                              break;

                        case META_KEY_SIGNATURE:
                              {
                              const char* keyTable[] = {
                                    "Ces", "Ges", "Des", "As", "Es", "Bes", "F",
                                    "C",
                                    "G", "D", "A", "E", "B", "Fis", "Cis"
                                    };
                              int key = (char)(_edata[0]) + 7;
                              if (key < 0 || key > 14) {
                                    qDebug("bad key signature %d", key);
                                    key = 0;
                                    }
                              QString sex(_edata[1] ? "Minor" : "Major");
                              QString keyName(keyTable[key]);
                              xml.tag(QString("Key tick=\"%1\" key=\"%2\" sex=\"%3\"").arg(ontime()).arg(_edata[0]).arg(_edata[1]),
                                 QString("%1 %2").arg(keyName).arg(sex));
                              }
                              break;

                        case META_TIME_SIGNATURE:
                              xml.tagE(QString("TimeSig tick=\"%1\" num=\"%2\" denom=\"%3\" metro=\"%4\" quarter=\"%5\"")
                                 .arg(ontime())
                                 .arg(int(_edata[0]))
                                 .arg(int(_edata[1]))
                                 .arg(int(_edata[2]))
                                 .arg(int(_edata[3])));
                              break;

                        case META_TEMPO:
                              {
                              unsigned tempo = _edata[2] + (_edata[1] << 8) + (_edata[0] << 16);
                              xml.tagE(QString("Tempo tick=\"%1\" value=\"%2\"").arg(ontime()).arg(tempo));
                              }
                              break;

                        default:
                              xml.stag(QString("Meta tick=\"%1\" type=\"%2\" len=\"%3\" name=\"%4\"")
                                 .arg(ontime()).arg(metaType()).arg(_len).arg(midiMetaName(metaType())));
                              xml.dump(_len, _edata);
                              xml.etag();
                              break;
                        }
                  break;
            }
      }

bool Event::operator==(const Event&) const
      {
      return false;           // TODO
      }

//---------------------------------------------------------
//    midi_meta_name
//---------------------------------------------------------

QString midiMetaName(int meta)
      {
      const char* s = "";
      switch (meta) {
            case 0:     s = "Sequence Number"; break;
            case 1:     s = "Text Event"; break;
            case 2:     s = "Copyright"; break;
            case 3:     s = "Sequence/Track Name"; break;
            case 4:     s = "Instrument Name"; break;
            case 5:     s = "Lyric"; break;
            case 6:     s = "Marker"; break;
            case 7:     s = "Cue Point"; break;
            case 8:
            case 9:
            case 0x0a:
            case 0x0b:
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:  s = "Text"; break;
            case 0x20:  s = "Channel Prefix"; break;
            case 0x21:  s = "Port Change"; break;
            case 0x2f:  s = "End of Track"; break;
            case META_TEMPO:  s = "Tempo"; break;
            case 0x54:  s = "SMPTE Offset"; break;
            case META_TIME_SIGNATURE:  s = "Time Signature"; break;
            case META_KEY_SIGNATURE:   s = "Key Signature"; break;
            case 0x74:                 s = "Sequencer-Specific1"; break;
            case 0x7f:                 s = "Sequencer-Specific2"; break;
            default:
                  break;
            }
      return QString(s);
      }

//---------------------------------------------------------
// insert
//---------------------------------------------------------

void EventList::insert(const Event& e)
      {
      int ontime = e.ontime();
      if (!isEmpty() && last().ontime() > ontime) {
            for (auto i = begin(); i != end(); ++i) {
                  if (i->ontime() > ontime) {
                        QList<Event>::insert(i, e);
                        return;
                        }
                  }
            }
      append(e);
      }
}

