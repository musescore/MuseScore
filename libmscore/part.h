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

#ifndef __PART_H__
#define __PART_H__

#include "mscore.h"
#include "instrument.h"
#include "text.h"

namespace Ms {

class XmlWriter;
class Staff;
class Score;
class InstrumentTemplate;

//---------------------------------------------------------
//   @@ Part
//   @P endTrack        int         (read only)
//   @P harmonyCount    int         (read only)
//   @P hasDrumStaff    bool        (read only)
//   @P hasPitchedStaff bool        (read only)
//   @P hasTabStaff     bool        (read only)
//   @P instrumentId    string      (read only)
//   @P longName        string
//   @P lyricCount      int         (read only)
//   @P midiChannel     int         (read only)
//   @P midiProgram     int         (read only)
//   @P mute            bool
//   @P partName        string      name of the part, used in the mixer
//   @P shortName       string
//   @P show            bool        check/set whether or not a part is shown
//   @P startTrack      int         (read only)
//   @P volume          int
//---------------------------------------------------------

class Part final : public ScoreElement {
      QString _partName;            ///< used in tracklist (mixer)
      InstrumentList _instruments;
      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

      static const int DEFAULT_COLOR = 0x3399ff;
      int _color;                   ///User specified color for helping to label parts

   public:
      Part(Score* = 0);
      void initFromInstrTemplate(const InstrumentTemplate*);
      virtual ElementType type() const override { return ElementType::PART; }

      void read(XmlReader&);
      bool readProperties(XmlReader&);
      void write(XmlWriter& xml) const;

      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }

      int startTrack() const;
      int endTrack() const;

      QString longName(int tick = -1) const;
      QString shortName(int tick = -1) const;
      QString instrumentName(int tick = -1) const;
      QString instrumentId(int tick = -1) const;

      const QList<StaffName>& longNames(int tick = -1) const  { return instrument(tick)->longNames();  }
      const QList<StaffName>& shortNames(int tick = -1) const { return instrument(tick)->shortNames(); }

      void setLongNames(QList<StaffName>& s, int tick = -1);
      void setShortNames(QList<StaffName>& s, int tick = -1);

      void setLongName(const QString& s);
      void setShortName(const QString& s);

      void setPlainLongName(const QString& s);
      void setPlainShortName(const QString& s);

      void setStaves(int);

      double volume() const;
      void setVolume(double volume);
      bool mute() const;
      void setMute(bool mute);

      double reverb() const;
      void setReverb(double);
      double chorus() const;
      void setChorus(double);
      double pan() const;
      void setPan(double pan);
      int midiProgram() const;
      void setMidiProgram(int, int bank = 0);

      int midiChannel() const;
      int midiPort() const;
      void setMidiChannel(int ch, int port = -1, int tick = -1);  // tick != -1 for InstrumentChange

      void insertStaff(Staff*, int idx);
      void removeStaff(Staff*);
      bool show() const                        { return _show;  }
      void setShow(bool val)                   { _show = val;   }

      Instrument* instrument(int tick = -1);
      const Instrument* instrument(int tick = -1) const;
      void setInstrument(Instrument*, int tick = -1);       // transfer ownership
      void setInstrument(const Instrument&&, int tick = -1);
      void setInstrument(const Instrument&, int tick = -1);
      void removeInstrument(int tick);
      const InstrumentList* instruments() const   { return &_instruments;       }

      void insertTime(int tick, int len);

      QString partName() const                 { return _partName; }
      void setPartName(const QString& s)       { _partName = s; }
      int color() const { return _color; }
      void setColor(int value) { _color = value; }

      QVariant getProperty(Pid) const override;
      bool setProperty(Pid, const QVariant&) override;

      int lyricCount();
      int harmonyCount();
      bool hasPitchedStaff();
      bool hasTabStaff();
      bool hasDrumStaff();
      };

}     // namespace Ms
#endif

