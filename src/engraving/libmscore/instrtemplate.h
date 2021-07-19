/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include "mscore.h"
#include "instrument.h"
#include "clef.h"
#include "stringdata.h"

namespace Ms {
class XmlWriter;
class Part;
class Staff;
class StringData;
class StaffType;
struct ScoreOrder;

//---------------------------------------------------------
//   InstrumentGenre
//---------------------------------------------------------

class InstrumentGenre
{
public:
    QString id;
    QString name;

    InstrumentGenre() {}
    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
};

//---------------------------------------------------------
//   InstrumentFamily
//---------------------------------------------------------

class InstrumentFamily
{
public:
    QString id;
    QString name;

    InstrumentFamily() {}
    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
};

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

class InstrumentTemplate
{
    int staves;               // 1 <= MAX_STAVES

public:
    QString id;
    QString trackName;
    StaffNameList longNames;     ///< shown on first system
    StaffNameList shortNames;    ///< shown on followup systems
    QString musicXMLid;          ///< used in MusicXML 3.0
    QString description;         ///< a longer description of the instrument

    int sequenceOrder = 0;

    Trait trait;

    char minPitchA;           // pitch range playable by an amateur
    char maxPitchA;
    char minPitchP;           // pitch range playable by professional
    char maxPitchP;

    Interval transpose;       // for transposing instruments

    StaffGroup staffGroup;
    const StaffType* staffTypePreset;
    bool useDrumset;
    Drumset* drumset;

    StringData stringData;

    QList<NamedEventList> midiActions;
    QList<MidiArticulation> articulation;
    QList<Channel> channel;
    QList<InstrumentGenre*> genres;       //; list of genres this instrument belongs to
    InstrumentFamily* family;             //; family the instrument belongs to

    ClefTypeList clefTypes[MAX_STAVES];
    int staffLines[MAX_STAVES];
    BracketType bracket[MAX_STAVES];              // bracket type (NO_BRACKET)
    int bracketSpan[MAX_STAVES];
    int barlineSpan[MAX_STAVES];
    bool smallStaff[MAX_STAVES];

    bool extended;            // belongs to extended instrument set if true

    bool singleNoteDynamics;

    InstrumentTemplate();
    InstrumentTemplate(const InstrumentTemplate&);
    ~InstrumentTemplate();
    void init(const InstrumentTemplate&);
    void linkGenre(const QString&);
    void addGenre(QList<InstrumentGenre*>);
    bool genreMember(const QString&);
    bool familyMember(const QString&);

    void setPitchRange(const QString& s, char* a, char* b) const;
    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
    int nstaves() const { return staves; }
    void setStaves(int val) { staves = val; }
    ClefTypeList clefType(int staffIdx) const;
};

//---------------------------------------------------------
//   InstrumentGroup
//---------------------------------------------------------

struct InstrumentGroup {
    QString id;
    QString name;
    bool extended;            // belongs to extended instruments set if true
    QList<InstrumentTemplate*> instrumentTemplates;
    void read(XmlReader&);
    void clear();

    InstrumentGroup() { extended = false; }
};

//---------------------------------------------------------
//   InstrumentIndex
//---------------------------------------------------------

struct InstrumentIndex {
    int groupIndex;
    int instrIndex;
    InstrumentTemplate* instrTemplate;

    InstrumentIndex(int g, int i, InstrumentTemplate* it)
        : groupIndex{g}, instrIndex{i}, instrTemplate{it} {}
};

extern QList<InstrumentGenre*> instrumentGenres;
extern QList<InstrumentFamily*> instrumentFamilies;
extern QList<MidiArticulation> articulation;
extern QList<InstrumentGroup*> instrumentGroups;
extern QList<ScoreOrder*> instrumentOrders;
extern void clearInstrumentTemplates();
extern bool loadInstrumentTemplates(const QString& instrTemplates);
extern InstrumentTemplate* searchTemplate(const QString& name);
extern InstrumentIndex searchTemplateIndexForTrackName(const QString& trackName);
extern InstrumentIndex searchTemplateIndexForId(const QString& id);
extern InstrumentTemplate* searchTemplateForMusicXmlId(const QString& mxmlId);
extern InstrumentTemplate* searchTemplateForInstrNameList(const QList<QString>& nameList);
extern InstrumentTemplate* searchTemplateForMidiProgram(int midiProgram, const bool useDrumKit = false);
extern InstrumentTemplate* guessTemplateByNameData(const QList<QString>& nameDataList);
extern InstrumentGroup* searchInstrumentGroup(const QString& name);
extern ClefType defaultClef(int patch);
}     // namespace Ms
#endif
