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

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include "mscore.h"
#include "instrument.h"
#include "clef.h"

namespace Ms {

class Xml;
class Part;
class Staff;
class StringData;

//---------------------------------------------------------
//   InstrumentGenre
//---------------------------------------------------------

class InstrumentGenre {
   public:
      QString id;
      QString name;

      InstrumentGenre() {}
      void write(Xml& xml) const;
      void write1(Xml& xml) const;
      void read(XmlReader&);
      };

//---------------------------------------------------------//   InstrumentTemplate
//---------------------------------------------------------

class InstrumentTemplate {
      int staves;             // 1 <= MAX_STAVES

   public:
      QString id;
      QString trackName;
      QList<StaffName> longNames;      ///< shown on first system
      QList<StaffName> shortNames;     ///< shown on followup systems
      QString musicXMLid;              ///< used in MusicXML 3.0
      QString description;             ///< a longer description of the instrument

      char minPitchA;         // pitch range playable by an amateur
      char maxPitchA;
      char minPitchP;         // pitch range playable by professional
      char maxPitchP;

      Interval transpose;     // for transposing instruments

      StaffGroup  staffGroup;
      int         staffTypePreset;
      bool useDrumset;
      Drumset* drumset;

      StringData* stringData;

      QList<NamedEventList>   midiActions;
      QList<MidiArticulation> articulation;
      QList<Channel>          channel;
      QList<InstrumentGenre*> genres;     //; list of genres this instrument belongs to

      ClefTypeList clefTypes[MAX_STAVES];
      int staffLines[MAX_STAVES];
      BracketType bracket[MAX_STAVES];            // bracket type (NO_BRACKET)
      int bracketSpan[MAX_STAVES];
      int barlineSpan[MAX_STAVES];
      bool smallStaff[MAX_STAVES];

      bool extended;          // belongs to extended instrument set if true

      InstrumentTemplate();
      InstrumentTemplate(const InstrumentTemplate&);
      ~InstrumentTemplate();
      void init(const InstrumentTemplate&);
      void linkGenre(const QString &);
      void addGenre(QList<InstrumentGenre *>);
      bool genreMember(const QString &);

      void setPitchRange(const QString& s, char* a, char* b) const;
      void write(Xml& xml) const;
      void write1(Xml& xml) const;
      void read(XmlReader&);
      int nstaves() const { return staves; }
      void setStaves(int val) { staves = val; }
      };

//---------------------------------------------------------
//   InstrumentGroup
//---------------------------------------------------------

struct InstrumentGroup {
      QString id;
      QString name;
      bool extended;          // belongs to extended instruments set if true
      QList<InstrumentTemplate*> instrumentTemplates;
      void read(XmlReader&);

      InstrumentGroup() { extended = false; }
      };

extern QList<InstrumentGenre *> instrumentGenres;
extern QList<InstrumentGroup*> instrumentGroups;
extern bool loadInstrumentTemplates(const QString& instrTemplates);
extern bool saveInstrumentTemplates(const QString& instrTemplates);
extern InstrumentTemplate* searchTemplate(const QString& name);

}     // namespace Ms
#endif

