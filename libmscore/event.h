//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: event.h 4876 2011-10-22 13:03:58Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EVENT_H__
#define __EVENT_H__

class Note;
// class MidiFile;
class Xml;

//---------------------------------------------------------
//   Midi Events
//---------------------------------------------------------

enum {
      ME_INVALID    = 0,
      ME_NOTEOFF    = 0x80,
      ME_NOTEON     = 0x90,
      ME_POLYAFTER  = 0xa0,
      ME_CONTROLLER = 0xb0,
      ME_PROGRAM    = 0xc0,
      ME_AFTERTOUCH = 0xd0,
      ME_PITCHBEND  = 0xe0,
      ME_SYSEX      = 0xf0,
      ME_META       = 0xff,
      ME_SONGPOS    = 0xf2,
      ME_ENDSYSEX   = 0xf7,
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,
      ME_SENSE      = 0xfe,   // active sense (used by yamaha)

      ME_NOTE       = 0x100,
      ME_CHORD      = 0x101,
      ME_TICK1      = 0x102,  // metronome tick akzent
      ME_TICK2      = 0x103,  // metronome tick
      };

//---------------------------------------------------------
//   Midi Meta Events
//---------------------------------------------------------

enum {
      META_SEQUENCE_NUMBER = 0,
      META_TEXT            = 1,
      META_COPYRIGHT       = 2,
      META_TRACK_NAME      = 3,
      META_INSTRUMENT_NAME = 4,
      META_LYRIC           = 5,
      META_MARKER          = 6,
      META_CUE_POINT       = 7,
      META_TITLE           = 8,     // mscore extension
      META_SUBTITLE        = 9,     // mscore extension
      META_COMPOSER        = 0xa,   // mscore extension
      META_TRANSLATOR      = 0xb,   // mscore extension
      META_POET            = 0xc,   // mscore extension
      META_TRACK_COMMENT   = 0xf,
      META_PORT_CHANGE     = 0x21,
      META_CHANNEL_PREFIX  = 0x22,
      META_EOT             = 0x2f,  // end of track
      META_TEMPO           = 0x51,
      META_TIME_SIGNATURE  = 0x58,
      META_KEY_SIGNATURE   = 0x59,
      };

//---------------------------------------------------------
//   Midi Controller
//---------------------------------------------------------

enum {
      CTRL_HBANK              = 0x00,
      CTRL_LBANK              = 0x20,

      CTRL_HDATA              = 0x06,
      CTRL_LDATA              = 0x26,

      CTRL_HNRPN              = 0x63,
      CTRL_LNRPN              = 0x62,

      CTRL_HRPN               = 0x65,
      CTRL_LRPN               = 0x64,

      CTRL_MODULATION         = 0x01,
      CTRL_PORTAMENTO_TIME    = 0x05,
      CTRL_VOLUME             = 0x07,
      CTRL_PANPOT             = 0x0a,
      CTRL_EXPRESSION         = 0x0b,
      CTRL_SUSTAIN            = 0x40,
      CTRL_PORTAMENTO         = 0x41,
      CTRL_SOSTENUTO          = 0x42,
      CTRL_SOFT_PEDAL         = 0x43,
      CTRL_HARMONIC_CONTENT   = 0x47,
      CTRL_RELEASE_TIME       = 0x48,
      CTRL_ATTACK_TIME        = 0x49,

      CTRL_BRIGHTNESS         = 0x4a,
      CTRL_PORTAMENTO_CONTROL = 0x54,
      CTRL_REVERB_SEND        = 0x5b,
      CTRL_CHORUS_SEND        = 0x5d,
      CTRL_VARIATION_SEND     = 0x5e,

      CTRL_ALL_SOUNDS_OFF     = 0x78, // 120
      CTRL_RESET_ALL_CTRL     = 0x79, // 121
      CTRL_LOCAL_OFF          = 0x7a, // 122

      // special midi events are mapped to internal
      // controller
      //
      CTRL_PROGRAM   = 0x40001,
      CTRL_PITCH     = 0x40002,
      CTRL_PRESS     = 0x40003,
      CTRL_POLYAFTER = 0x40004,
      };

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class EventData;

class Event {
      QSharedDataPointer<EventData> d;

   public:
      Event();
      Event(const Event&);
      Event(int t);
      ~Event();
      Event& operator=(const Event&);
      bool operator==(const Event&) const;

      void write(Xml&) const;
      void dump() const;

      bool isChannelEvent() const;

      int noquantOntime() const;
      void setNoquantOntime(int v);
      int noquantDuration() const;
      void setNoquantDuration(int v);

      int type() const;
      void setType(int v);
      int ontime() const;
      void setOntime(int v);
      int channel() const;
      void setChannel(int c);
      int dataA() const;
      int dataB() const;
      void setDataA(int v);
      void setDataB(int v);
      int pitch() const;
      void setPitch(int v);
      int velo() const;
      void setVelo(int v);
      int controller() const;
      void setController(int val);
      int value() const;
      void setValue(int v);
      int duration() const;
      void setDuration(int v);
      int voice() const;
      void setVoice(int val);
      int offtime() const;
      QList<Event>& notes();
      const uchar* data() const;
      void setData(uchar* d);
      int len() const;
      void setLen(int l);
      int metaType() const;
      void setMetaType(int v);
      int tpc() const;
      void setTpc(int v);
      const Note* note() const;
      void setNote(const Note* v);
      qreal tuning() const;
      void setTuning(qreal v);
      };

//---------------------------------------------------------
//   EventList
//   EventMap
//---------------------------------------------------------

class EventList : public QList<Event> {
   public:
      void insert(const Event&);
      void insertNote(int channel, Note*);
      };

class EventMap : public QMap<int, Event> {};

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

extern QString midiMetaName(int meta);


#endif

