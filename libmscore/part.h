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

class Xml;
class Staff;
class Score;
class InstrumentTemplate;

//---------------------------------------------------------
//   @@ Part
//   @P partName   QString  name of the part, used in the mixer
//   @P show       bool     check/set whether or not a part is shown
//   @P longName   QString
//   @P shortName  QString
//   @P volume     int
//   @P mute       bool
//   @P endTrack   int      (read only)
//   @P startTrack int      (read only)
//---------------------------------------------------------

class Part : public QObject {
      Q_OBJECT

      Q_PROPERTY(QString partName READ partName WRITE setPartName)
      Q_PROPERTY(bool show READ show WRITE setShow)
      Q_PROPERTY(QString longName READ longName WRITE setLongName)
      Q_PROPERTY(QString shortName READ shortName WRITE setShortName)
      Q_PROPERTY(int volume READ volume WRITE setVolume)
      Q_PROPERTY(bool mute READ mute WRITE setMute)
      Q_PROPERTY(int endTrack READ endTrack)
      Q_PROPERTY(int startTrack READ startTrack)

      Score* _score;

      QString _partName;           ///< used in tracklist (mixer)
      InstrumentList _instrList;

      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score* = 0);
      void initFromInstrTemplate(const InstrumentTemplate*);

      void read(XmlReader&);
      void read114(XmlReader&);
      void write(Xml& xml) const;

      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }

      int startTrack() const;
      int endTrack() const;

      QString longName(int tick = 0) const;
      QString shortName(int tick = 0) const;
      QString instrumentName(int tick = 0) const;

      const QList<StaffName>& longNames(int tick = 0) const  { return instr(tick)->longNames();  }
      const QList<StaffName>& shortNames(int tick = 0) const { return instr(tick)->shortNames(); }

      void setLongNames(QList<StaffName>& s, int tick = 0);
      void setShortNames(QList<StaffName>& s, int tick = 0);

      void setLongName(const QString& s);
      void setShortName(const QString& s);

      void setStaves(int);

      int volume() const;
      void setVolume(int volume);
      bool mute() const;
      void setMute(bool mute);

      int reverb() const;
      int chorus() const;
      int pan() const;
      void setPan(int pan);
      int midiProgram() const;
      void setMidiProgram(int, int bank = 0);

      int midiChannel() const;
      void setMidiChannel(int) const;

      void insertStaff(Staff*, int idx);
      void removeStaff(Staff*);
      bool show() const                        { return _show;  }
      void setShow(bool val)                   { _show = val;   }
      Score* score() const                     { return _score; }

      Instrument* instr(int tick = 0);
      const Instrument* instr(int tick = 0) const;
      void setInstrument(const Instrument&, int tick = 0);
      void removeInstrument(int tick);

      QString partName() const                 { return _partName; }
      void setPartName(const QString& s)       { _partName = s; }
      InstrumentList* instrList()              { return &_instrList;       }

      QVariant getProperty(int id) const;
      void setProperty(int id, const QVariant& property);
      };


}     // namespace Ms
#endif

