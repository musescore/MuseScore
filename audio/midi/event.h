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

#ifndef __EVENT_H__
#define __EVENT_H__

#include <map>

namespace Ms {

class Note;
class Harmony;
class XmlWriter;
class Score;

enum class BeatType : char;

// 4 is the default for the majority of synthesisers, aka VSTis
const int PITCH_BEND_SENSITIVITY = 4;

const int MIDI_ON_SIGNAL = 127;

//---------------------------------------------------------
//   Event types
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

      ME_NOTE       = 0x1,
      ME_CHORD      = 0x2,
      ME_TICK1      = 0x3,  // metronome tick akzent
      ME_TICK2      = 0x4,  // metronome tick
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
      META_PROGRAM_NAME    = 8, // MIDI Meta Events 8 and 9 were defined as above by the MMA in 1998
      META_DEVICE_NAME     = 9, // It is therefore necessary to redefine MuseScore's private meta events
      META_TRACK_COMMENT   = 0xf, // Using the block starting 0x10 seems sensible as that is currently clear
      META_TITLE           = 0x10,     // mscore extension
      META_SUBTITLE        = 0x11,     // mscore extension
      META_COMPOSER        = 0x12,   // mscore extension
      META_TRANSLATOR      = 0x13,   // mscore extension
      META_POET            = 0x14,   // mscore extension
      META_PORT_CHANGE     = 0x21,
      META_CHANNEL_PREFIX  = 0x22,
      META_EOT             = 0x2f,  // end of track
      META_TEMPO           = 0x51,
      META_TIME_SIGNATURE  = 0x58,
      META_KEY_SIGNATURE   = 0x59,
      META_SPECIFIC        = 0x7F   // sequencer specific
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
      CTRL_BREATH             = 0x02,
      CTRL_FOOT               = 0x04,
      CTRL_PORTAMENTO_TIME_MSB= 0x05,
      CTRL_VOLUME             = 0x07,
      CTRL_PANPOT             = 0x0a,
      CTRL_EXPRESSION         = 0x0b,
      CTRL_PORTAMENTO_TIME_LSB= 0x25,
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
      CTRL_ALL_NOTES_OFF      = 0x7b,  // 123

      // special midi events are mapped to internal
      // controller
      //
      CTRL_PROGRAM   = 0x81,
      /*             = 0x82,*/
      CTRL_PRESS     = 0x83,
      CTRL_POLYAFTER = 0x84
      };

//---------------------------------------------------------
//   MidiCoreEvent
//---------------------------------------------------------

class MidiCoreEvent {
   protected:
      uchar _type    = 0;
      uchar _channel = 0;
      uchar _a       = 0;
      uchar _b       = 0;

   public:
      MidiCoreEvent() {}
      MidiCoreEvent(uchar t, uchar c, uchar a, uchar b)
         : _type(t), _channel(c), _a(a), _b(b) {}

      void set(uchar t, uchar c, uchar a, uchar b) {
            _type    = t;
            _channel = c;
            _a       = a;
            _b       = b;
            }

      uchar type() const             { return _type;    }
      void  setType(uchar t)         { _type = t;       }
      uchar channel() const          { return _channel; }
      void  setChannel(uchar c)      { _channel = c;    }

      int dataA() const              { return _a; }
      int pitch() const              { return _a; }
      int controller() const         { return _a; }

      void setDataA(int v)           { _a = v; }
      void setPitch(int v)           { _a = v; }
      void setController(int v)      { _a = v; }

      int dataB() const              { return _b; }
      int velo() const               { return _b; }
      int value() const              { return _b; }

      void setDataB(int v)           { _b = v; }
      void setVelo(int v)            { _b = v; }
      void setValue(int v)           { _b = v; }

      void setData(int a, int b)        { _a = a; _b = b; }
      void setData(int t, int a, int b) { _type = t; _a = a; _b = b; }

      bool isChannelEvent() const;
      void write(XmlWriter&) const;
      bool operator==(const MidiCoreEvent& e) const {
            return e._type == _type && e._channel == _channel && e._a == _a && e._b == _b;
            }
      };

//---------------------------------------------------------
//   MidiEvent
//---------------------------------------------------------

class MidiEvent : public MidiCoreEvent {

   protected:
      std::vector<uchar> _edata;
      int _len { 0 };
      int _metaType { 0 };

   public:
      MidiEvent() {}
      MidiEvent(uchar t, uchar c, uchar a, uchar b)
         : MidiCoreEvent(t, c, a, b), _edata(0), _len(0) {}

      const uchar* edata() const            { return _edata.data(); }
      void setEData(std::vector<uchar>&& d) { _edata = d; }
      int len() const                       { return _len; }
      void setLen(int l)                    { _len = l; }
      int metaType() const                  { return _metaType; }
      void setMetaType(int v)               { _metaType = v; }
      };

//---------------------------------------------------------
//   PlayEvent
//    interface to Synthesizer
//---------------------------------------------------------

class PlayEvent : public MidiCoreEvent {

   protected:
      float _tuning = .0f;

   public:
      PlayEvent() : MidiCoreEvent() {}
      PlayEvent(const MidiCoreEvent& e) : MidiCoreEvent(e) {}
      PlayEvent(uchar t, uchar c, uchar a, uchar b)
         : MidiCoreEvent(t, c, a, b) {}
      float tuning() const           { return _tuning;  }
      void setTuning(float v)        { _tuning = v;     }
      };

//---------------------------------------------------------
//   NPlayEvent
//    used for Sequencer interface
//---------------------------------------------------------

class NPlayEvent : public PlayEvent {
      const Note* _note{nullptr};
      const Harmony* _harmony{nullptr};
      int _origin = -1;
      int _discard = 0;
      bool _portamento = false;

   public:
      NPlayEvent() : PlayEvent() {}
      NPlayEvent(uchar t, uchar c, uchar a, uchar b)
         : PlayEvent(t, c, a, b) {}
      NPlayEvent(const MidiCoreEvent& e) : PlayEvent(e) {}
      NPlayEvent(BeatType beatType);

      const Note* note() const            { return _note;    }
      void setNote(const Note* v)         { _note = v;       }
      const Harmony* harmony() const      { return _harmony; }
      void setHarmony(const Harmony* v)   { _harmony = v;    }

      int getOriginatingStaff() const { return _origin; }
      void setOriginatingStaff(int i) { _origin = i; }
      void setDiscard(int d) { _discard = d; }
      int discard() const { return _discard; }
      bool isMuted() const;
      void setPortamento(bool p) { _portamento = p; }
      bool portamento() const { 
            return (_portamento == true || 
                  (this->type() == ME_CONTROLLER && 
                  (this->controller() == CTRL_PORTAMENTO || this->controller() == CTRL_PORTAMENTO_CONTROL || 
                        this->controller() == CTRL_PORTAMENTO_TIME_MSB ||  this->controller() == CTRL_PORTAMENTO_TIME_LSB))); }
      };

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class Event : public PlayEvent {
      int _ontime;
      int _noquantOntime;
      int _noquantDuration;
      int _duration;
      int _tpc;               // tonal pitch class
      int _voice;
      QList<Event> _notes;
      uchar* _edata;           // always zero terminated (_data[_len] == 0; )
      int _len;
      int _metaType;
      const Note* _note;

   public:
      Event();
      Event(const Event&);
      Event(int t);
      ~Event();
      bool operator==(const Event&) const;

      void write(XmlWriter&) const;
      void dump() const;


      int noquantOntime() const      { return _noquantOntime;   }
      void setNoquantOntime(int v)   { _noquantOntime = v;      }
      int noquantDuration() const    { return _noquantDuration; }
      void setNoquantDuration(int v) { _noquantDuration = v;    }

      int ontime() const             { return _ontime; }
      void setOntime(int v)          { _ontime = v; }

      int duration() const           { return _duration; }
      void setDuration(int v)        { _duration = v; }
      int voice() const              { return _voice; }
      void setVoice(int val)         { _voice = val; }
      int offtime() const            { return _ontime + _duration; }
      QList<Event>& notes()          { return _notes; }
      const uchar* edata() const     { return _edata; }
      void setEData(uchar* d)        { _edata = d; }
      int len() const                { return _len; }
      void setLen(int l)             { _len = l; }
      int metaType() const           { return _metaType; }
      void setMetaType(int v)        { _metaType = v; }
      int tpc() const                { return _tpc; }
      void setTpc(int v)             { _tpc = v; }
      const Note* note() const       { return _note; }
      void setNote(const Note* v)    { _note = v; }
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

class EventMap : public std::multimap<int, NPlayEvent> {
      int _highestChannel = 15;
   public:
      void fixupMIDI();
      void registerChannel(int c) { if (c > _highestChannel) _highestChannel = c; }
      };

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

extern QString midiMetaName(int meta);

}
#endif

