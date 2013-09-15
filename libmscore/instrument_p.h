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

#ifndef __INSTRUMENT_P_H__
#define __INSTRUMENT_P_H__

#include "instrument.h"

namespace Ms {

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class InstrumentData : public QSharedData {
      QList<StaffNameDoc> _longNames;
      QList<StaffNameDoc> _shortNames;
      QString _trackName;

      char _minPitchA, _maxPitchA, _minPitchP, _maxPitchP;
      Interval _transpose;

      bool       _useDrumset;
      Drumset*   _drumset;
      StringData* _stringData;

      QList<NamedEventList>   _midiActions;
      QList<MidiArticulation> _articulation;
      QList<Channel> _channel;      // at least one entry

   public:
      InstrumentData();
      InstrumentData(const InstrumentData&);
      ~InstrumentData();

      void read(XmlReader&);
      void write(Xml& xml) const;
      NamedEventList* midiAction(const QString& s, int channel) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);
      void updateGateTime(int* gateTime, int channelIdx, const QString& name);

      bool operator==(const InstrumentData&) const;

      void setMinPitchP(int v)                               { _minPitchP = v;     }
      void setMaxPitchP(int v)                               { _maxPitchP = v;     }
      void setMinPitchA(int v)                               { _minPitchA = v;     }
      void setMaxPitchA(int v)                               { _maxPitchA = v;     }
      Interval transpose() const                             { return _transpose; }
      void setTranspose(const Interval& v)                   { _transpose = v; }

      void setDrumset(Drumset* ds);       // drumset is now owned by Instrument
      Drumset* drumset() const                               { return _drumset;    }
      bool useDrumset() const                                { return _useDrumset; }
      void setUseDrumset(bool val);
      void setAmateurPitchRange(int a, int b)                { _minPitchA = a; _maxPitchA = b; }
      void setProfessionalPitchRange(int a, int b)           { _minPitchP = a; _maxPitchP = b; }
      Channel& channel(int idx)                              { return _channel[idx]; }
      const Channel& channel(int idx) const                  { return _channel[idx]; }

      const QList<NamedEventList>& midiActions() const       { return _midiActions; }
      const QList<MidiArticulation>& articulation() const    { return _articulation; }
      const QList<Channel>& channel() const                  { return _channel; }

      void setMidiActions(const QList<NamedEventList>& l)    { _midiActions = l;  }
      void setArticulation(const QList<MidiArticulation>& l) { _articulation = l; }
      void setChannel(const QList<Channel>& l)               { _channel = l;      }
      void setChannel(int i, const Channel& c)               { _channel[i] = c;   }
      StringData* stringData() const;
      void setStringData(StringData* t);    // move ownership of tablature to Instrument

      void setLongName(const QTextDocumentFragment& f);
      void setShortName(const QTextDocumentFragment& f);

      void addLongName(const StaffNameDoc& f);
      void addShortName(const StaffNameDoc& f);


      friend class Instrument;
      };


}     // namespace Ms
#endif

