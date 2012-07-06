//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: part.h 5335 2012-02-17 13:51:54Z lasconic $
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

class Xml;
class Staff;
class Score;
struct InstrumentTemplate;

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part : public QObject {
      Q_OBJECT

      Score* _score;

      QString _partName;           ///< used in tracklist (mixer)
      InstrumentList _instrList;

      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score* = 0);
      void initFromInstrTemplate(const InstrumentTemplate*);

      void read(const QDomElement&);
      void write(Xml& xml) const;
      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }

      QTextDocumentFragment longName(int tick = 0) const;
      QTextDocumentFragment shortName(int tick = 0) const;
      QString instrumentName(int tick = 0) const;

      const QList<StaffNameDoc>& longNames(int tick = 0) const  { return instr(tick)->longNames();  }
      const QList<StaffNameDoc>& shortNames(int tick = 0) const { return instr(tick)->shortNames(); }

      void setLongNames(QList<StaffNameDoc>& s, int tick = 0);
      void setShortNames(QList<StaffNameDoc>& s, int tick = 0);

      void setLongName(const QString& s);
      void setShortName(const QString& s);

      void setStaves(int);

      int volume() const;
      void setVolume(int volume);
      int reverb() const;
      int chorus() const;
      int pan() const;
      void setPan(int pan);
      int midiProgram() const;
      void setMidiProgram(int, int bank = 0);

      int midiChannel() const;
      void setMidiChannel(int) const;

      void insertStaff(Staff*);
      void removeStaff(Staff*);
      bool show() const                        { return _show;        }
      void setShow(bool val);
      Score* score() const                     { return _score; }

      Instrument* instr(int tick = 0);
      const Instrument* instr(int tick = 0) const;
      void setInstrument(const Instrument&, int tick = 0);
      void removeInstrument(int tick);

      QString partName() const                 { return _partName; }
      void setPartName(const QString& s)       { _partName = s; }
      InstrumentList* instrList()              { return &_instrList;       }
      };

#endif

