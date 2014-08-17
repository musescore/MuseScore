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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "mscore.h"
#include "synthesizer/event.h"
#include "interval.h"
#include "clef.h"

namespace Ms {

class InstrumentTemplate;
class Xml;
class XmlReader;
class Drumset;
class StringData;

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

struct StaffName {
      QString name;
      int pos;          // even number -> between staves

      StaffName() {}
      StaffName(const QString& s, int p=0) : name(s), pos(p) {}
      bool operator==(const StaffName&) const;
      void read(XmlReader&);
      void write(Xml& xml, const char* name) const;
      };

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
      QString name;
      QString descr;
      std::vector<MidiCoreEvent> events;

      void write(Xml&, const QString& name) const;
      void read(XmlReader&);
      bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
      };

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
      QString name;
      QString descr;
      int velocity;           // velocity change: -100% - +100%
      int gateTime;           // gate time change: -100% - +100%
      void write(Xml&) const;
      void read(XmlReader&);

      MidiArticulation() {}
      MidiArticulation(const QString& n, const QString& d, int v, int g) : name(n), descr(d), velocity(v), gateTime(g) {}
      bool operator==(const MidiArticulation& i) const;
      };

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

struct Channel {
      // this are the indexes of controllers which are always present in
      // Channel init EventList (maybe zero)

      enum class A : char {
            HBANK, LBANK, PROGRAM, VOLUME, PAN, CHORUS, REVERB,
            INIT_COUNT
            };
      QString name;
      QString descr;
      int channel;      // mscore channel number, mapped to midi port/channel
      mutable std::vector<MidiCoreEvent> init;

      QString synti;
      int program;     // current values as shown in mixer
      int bank;        // initialized from "init"
      char volume;
      char pan;
      char chorus;
      char reverb;

      bool mute;
      bool solo;
      bool soloMute;

      QList<NamedEventList> midiActions;
      QList<MidiArticulation> articulation;

      Channel();
      void write(Xml&) const;
      void read(XmlReader&);
      void updateInitList() const;
      bool operator==(const Channel& c) { return (name == c.name) && (channel == c.channel); }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

enum class DrumsetKind : char { NONE, DEFAULT_DRUMS, GUITAR_PRO };

class InstrumentData;

class Instrument {
      QSharedDataPointer<InstrumentData> d;

   public:
      Instrument();
      Instrument(const Instrument&);
      ~Instrument();
      Instrument& operator=(const Instrument&);
      bool operator==(const Instrument&) const;

      void read(XmlReader&);
      void write(Xml& xml) const;
      NamedEventList* midiAction(const QString& s, int channel) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);
      void updateGateTime(int* gateTime, int channel, const QString& name);

      int minPitchP() const;
      int maxPitchP() const;
      int minPitchA() const;
      int maxPitchA() const;
      void setMinPitchP(int v);
      void setMaxPitchP(int v);
      void setMinPitchA(int v);
      void setMaxPitchA(int v);
      Interval transpose() const;
      void setTranspose(const Interval& v);

      void setDrumset(Drumset* ds);       // drumset is now owned by Instrument
      Drumset* drumset() const;
      DrumsetKind useDrumset() const;
      void setUseDrumset(DrumsetKind val);
      void setAmateurPitchRange(int a, int b);
      void setProfessionalPitchRange(int a, int b);
      Channel& channel(int idx);
      const Channel& channel(int idx) const;
      ClefTypeList clefType(int staffIdx) const;
      void setClefType(int staffIdx, const ClefTypeList&);

      const QList<NamedEventList>& midiActions() const;
      const QList<MidiArticulation>& articulation() const;
      const QList<Channel>& channel() const;

      void setMidiActions(const QList<NamedEventList>& l);
      void setArticulation(const QList<MidiArticulation>& l);
      void setChannel(const QList<Channel>& l);
      void setChannel(int i, const Channel& c);
      const StringData* stringData() const;
      void setStringData(const StringData&);
      static Instrument fromTemplate(const InstrumentTemplate*);

      const QList<StaffName>& longNames() const;
      const QList<StaffName>& shortNames() const;
      QList<StaffName>& longNames();
      QList<StaffName>& shortNames();
      void setLongName(const QString&);
      void setShortName(const QString&);
      void addLongName(const StaffName& f);
      void addShortName(const StaffName& f);

      QString trackName() const;
      void setTrackName(const QString&);
      };

//---------------------------------------------------------
//   InstrumentList
//---------------------------------------------------------

class InstrumentList : public std::map<const int, Instrument> {
      static Instrument defaultInstrument;
   public:
      InstrumentList() {}
      const Instrument& instrument(int tick) const;
      Instrument& instrument(int tick);
      void setInstrument(const Instrument&, int tick);
      };


}     // namespace Ms
#endif

