//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: instrtemplate.h 5377 2012-02-25 10:24:05Z wschweer $
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

class Xml;
class Part;
class Staff;
class Tablature;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

class InstrumentTemplate {
      int staves;             // 1 <= MAX_STAVES

   public:
      QString id;
      QString trackName;
      QList<StaffName> longNames;      ///< shown on first system
      QList<StaffName> shortNames;     ///< shown on followup systems

      char minPitchA;         // pitch range playable by an amateur
      char maxPitchA;
      char minPitchP;         // pitch range playable by professional
      char maxPitchP;

      Interval transpose;     // for transposing instruments

      bool useDrumset;
      Drumset* drumset;

      bool useTablature;
      Tablature* tablature;

      QList<NamedEventList>   midiActions;
      QList<MidiArticulation> articulation;
      QList<Channel>          channel;

      ClefType clefIdx[MAX_STAVES];
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

      void setPitchRange(const QString& s, char* a, char* b) const;
      void write(Xml& xml) const;
      void write1(Xml& xml) const;
      void read(const QDomElement&);
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
      void read(const QDomElement&);

      InstrumentGroup() { extended = false; }
      };

extern QList<InstrumentGroup*> instrumentGroups;
extern bool loadInstrumentTemplates(const QString& instrTemplates);
extern bool saveInstrumentTemplates(const QString& instrTemplates);
extern InstrumentTemplate* searchTemplate(const QString& name);
#endif

