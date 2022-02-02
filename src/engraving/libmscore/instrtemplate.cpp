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

#include "instrtemplate.h"

#include "translation.h"
#include "style/style.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "bracket.h"
#include "drumset.h"
#include "stafftype.h"
#include "stringdata.h"
#include "utils.h"
#include "scoreorder.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
QList<InstrumentGroup*> instrumentGroups;
QList<MidiArticulation> articulation;                 // global articulations
QList<InstrumentGenre*> instrumentGenres;
QList<InstrumentFamily*> instrumentFamilies;
QList<ScoreOrder> instrumentOrders;

//---------------------------------------------------------
//   InstrumentIndex
//---------------------------------------------------------

InstrumentIndex::InstrumentIndex(int g, int i, InstrumentTemplate* it)
    : groupIndex{g}, instrIndex{i}, instrTemplate{it}
{
    templateCount = 0;
    for (InstrumentGroup* ig : qAsConst(instrumentGroups)) {
        templateCount += ig->instrumentTemplates.size();
    }
}

//---------------------------------------------------------
//   searchInstrumentGenre
//---------------------------------------------------------

static InstrumentGenre* searchInstrumentGenre(const QString& genre)
{
    for (InstrumentGenre* ig : qAsConst(instrumentGenres)) {
        if (ig->id == genre) {
            return ig;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   searchInstrumentFamily
//---------------------------------------------------------

static InstrumentFamily* searchInstrumentFamily(const QString& name)
{
    for (InstrumentFamily* fam : qAsConst(instrumentFamilies)) {
        if (fam->id == name) {
            return fam;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   searchInstrumentGroup
//---------------------------------------------------------

InstrumentGroup* searchInstrumentGroup(const QString& name)
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        if (g->id == name) {
            return g;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   searchArticulation
//---------------------------------------------------------

static MidiArticulation searchArticulation(const QString& name)
{
    for (MidiArticulation a : qAsConst(articulation)) {
        if (a.name == name) {
            return a;
        }
    }
    return MidiArticulation();
}

//---------------------------------------------------------
//   readStaffIdx
//---------------------------------------------------------

static int readStaffIdx(XmlReader& e)
{
    int idx = e.intAttribute("staff", 1) - 1;
    if (idx >= MAX_STAVES) {
        idx = MAX_STAVES - 1;
    }
    if (idx < 0) {
        idx = 0;
    }
    return idx;
}

static TraitType traitTypeFromString(const QString& str)
{
    static const QMap<QString, TraitType> types {
        { "transposition", TraitType::Transposition },
        { "tuning", TraitType::Tuning },
        { "course", TraitType::Course }
    };

    return types.value(str.toLower(), TraitType::Unknown);
}

//---------------------------------------------------------
//   read InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::read(XmlReader& e)
{
    id       = e.attribute("id");
    name     = qtrc("InstrumentsXML", e.attribute("name").toUtf8().data());
    extended = e.intAttribute("extended", 0);

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "instrument" || tag == "Instrument") {
            QString sid = e.attribute("id");
            InstrumentTemplate* t = searchTemplate(sid);
            if (t == 0) {
                t = new InstrumentTemplate;
                t->articulation.append(articulation);             // init with global articulation
                t->sequenceOrder = instrumentTemplates.size();
                instrumentTemplates.append(t);
            }
            t->read(e);
        } else if (tag == "ref") {
            InstrumentTemplate* ttt = searchTemplate(e.readElementText());
            if (ttt) {
                InstrumentTemplate* t = new InstrumentTemplate(*ttt);
                instrumentTemplates.append(t);
            } else {
                qDebug("instrument reference not found <%s>", e.text().toUtf8().data());
            }
        } else if (tag == "name") {
            name = qtrc("InstrumentsXML", e.readElementText().toUtf8().data());
        } else if (tag == "extended") {
            extended = e.readInt();
        } else {
            e.unknown();
        }
    }
    if (id.isEmpty()) {
        id = name.toLower().replace(" ", "-");
    }
}

//---------------------------------------------------------
//   clear InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::clear()
{
    qDeleteAll(instrumentTemplates);
    instrumentTemplates.clear();
}

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
{
    staffCount        = 1;
    minPitchA          = 0;
    maxPitchA          = 127;
    minPitchP          = 0;
    maxPitchP          = 127;
    staffGroup         = StaffGroup::STANDARD;
    staffTypePreset    = 0;
    useDrumset         = false;
    drumset            = 0;
    extended           = false;
    singleNoteDynamics = true;
    family             = nullptr;

    for (int i = 0; i < MAX_STAVES; ++i) {
        clefTypes[i]._concertClef = ClefType::G;
        clefTypes[i]._transposingClef = ClefType::G;
        staffLines[i]  = 5;
        smallStaff[i]  = false;
        bracket[i]     = BracketType::NO_BRACKET;
        bracketSpan[i] = 0;
        barlineSpan[i] = false;
    }
    transpose.diatonic   = 0;
    transpose.chromatic  = 0;
}

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
{
    init(t);
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentTemplate::init(const InstrumentTemplate& t)
{
    id = t.id;
    trackName = t.trackName;
    longNames = t.longNames;
    shortNames = t.shortNames;
    description = t.description;
    musicXMLid = t.musicXMLid;
    staffCount = t.staffCount;
    extended = t.extended;
    minPitchA = t.minPitchA;
    maxPitchA = t.maxPitchA;
    minPitchP = t.minPitchP;
    maxPitchP = t.maxPitchP;
    transpose = t.transpose;
    staffGroup = t.staffGroup;
    staffTypePreset = t.staffTypePreset;
    useDrumset = t.useDrumset;
    drumset = t.drumset ? new Drumset(*t.drumset) : nullptr;
    stringData = t.stringData;
    midiActions = t.midiActions;
    genres = t.genres;
    channel = t.channel;
    family = t.family;
    singleNoteDynamics = t.singleNoteDynamics;
    sequenceOrder = t.sequenceOrder;
    trait = t.trait;
    groupId = t.groupId;

    for (int i = 0; i < MAX_STAVES; ++i) {
        clefTypes[i]   = t.clefTypes[i];
        staffLines[i]  = t.staffLines[i];
        smallStaff[i]  = t.smallStaff[i];
        bracket[i]     = t.bracket[i];
        bracketSpan[i] = t.bracketSpan[i];
        barlineSpan[i] = t.barlineSpan[i];
    }
}

InstrumentTemplate::~InstrumentTemplate()
{
    delete drumset;
    drumset = nullptr;
}

InstrumentTemplate& InstrumentTemplate::operator=(const InstrumentTemplate& templ)
{
    init(templ);
    return *this;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(XmlWriter& xml) const
{
    xml.startObject(QString("Instrument id=\"%1\"").arg(id));
    longNames.write(xml, "longName");
    shortNames.write(xml, "shortName");

    if (longNames.size() > 1) {
        xml.tag("trackName", trackName);
    }
    xml.tag("description", description);
    xml.tag("musicXMLid", musicXMLid);
    if (extended) {
        xml.tag("extended", extended);
    }
    stringData.write(xml);
    if (staffCount > 1) {
        xml.tag("staves", staffCount);
    }
    for (int i = 0; i < staffCount; ++i) {
        if (clefTypes[i]._concertClef == clefTypes[i]._transposingClef) {
            QString tag = TConv::toXml(clefTypes[i]._concertClef);
            if (i) {
                xml.tag(QString("clef staff=\"%1\"").arg(i + 1), tag);
            } else {
                xml.tag("clef", tag);
            }
        } else {
            QString tag1 = TConv::toXml(clefTypes[i]._concertClef);
            QString tag2 = TConv::toXml(clefTypes[i]._transposingClef);
            if (i) {
                xml.tag(QString("concertClef staff=\"%1\"").arg(i + 1), tag1);
                xml.tag(QString("transposingClef staff=\"%1\"").arg(i + 1), tag2);
            } else {
                xml.tag("concertClef", tag1);
                xml.tag("transposingClef", tag2);
            }
        }
        if (staffLines[i] != 5) {
            if (i) {
                xml.tag(QString("stafflines staff=\"%1\"").arg(i + 1), staffLines[i]);
            } else {
                xml.tag("stafflines", staffLines[i]);
            }
        }
        if (smallStaff[i]) {
            if (i) {
                xml.tag(QString("smallStaff staff=\"%1\"").arg(i + 1), smallStaff[i]);
            } else {
                xml.tag("smallStaff", smallStaff[i]);
            }
        }

        if (bracket[i] != BracketType::NO_BRACKET) {
            if (i) {
                xml.tag(QString("bracket staff=\"%1\"").arg(i + 1), int(bracket[i]));
            } else {
                xml.tag("bracket", int(bracket[i]));
            }
        }
        if (bracketSpan[i] != 0) {
            if (i) {
                xml.tag(QString("bracketSpan staff=\"%1\"").arg(i + 1), bracketSpan[i]);
            } else {
                xml.tag("bracketSpan", bracketSpan[i]);
            }
        }
        if (barlineSpan[i]) {
            if (i) {
                xml.tag(QString("barlineSpan staff=\"%1\"").arg(i + 1), barlineSpan[i]);
            } else {
                xml.tag("barlineSpan", barlineSpan[i]);
            }
        }
    }
    if (minPitchA != 0 || maxPitchA != 127) {
        xml.tag("aPitchRange", QString("%1-%2").arg(int(minPitchA)).arg(int(maxPitchA)));
    }
    if (minPitchP != 0 || maxPitchP != 127) {
        xml.tag("pPitchRange", QString("%1-%2").arg(int(minPitchP)).arg(int(maxPitchP)));
    }
    if (transpose.diatonic) {
        xml.tag("transposeDiatonic", transpose.diatonic);
    }
    if (transpose.chromatic) {
        xml.tag("transposeChromatic", transpose.chromatic);
    }
    if (useDrumset) {
        xml.tag("drumset", int(useDrumset));
    }
    if (drumset) {
        drumset->save(xml);
    }

    if (!singleNoteDynamics) {      // default is true
        xml.tag("singleNoteDynamics", singleNoteDynamics);
    }

    for (const NamedEventList& a : midiActions) {
        a.write(xml, "MidiAction");
    }
    for (const Channel& a : channel) {
        a.write(xml, nullptr);
    }
    for (const MidiArticulation& ma : articulation) {
        bool isGlobal = false;
        for (const MidiArticulation& ga : qAsConst(Ms::articulation)) {
            if (ma == ga) {
                isGlobal = true;
                break;
            }
        }
        if (!isGlobal) {
            ma.write(xml);
        }
    }
    if (family) {
        xml.tag("family", family->id);
    }
    xml.endObject();
}

//---------------------------------------------------------
//   write1
//    output only translatable names
//---------------------------------------------------------

void InstrumentTemplate::write1(XmlWriter& xml) const
{
    xml.startObject(QString("Instrument id=\"%1\"").arg(id));
    longNames.write(xml, "longName");
    shortNames.write(xml, "shortName");
    if (longNames.size() > 1) {
        xml.tag("trackName", trackName);
    }
    xml.tag("description", description);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(XmlReader& e)
{
    id = e.attribute("id");

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "longName" || tag == "name") {                   // "name" is obsolete
            int pos = e.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = longNames.begin(); i != longNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    longNames.erase(i);
                    break;
                }
            }
            longNames.append(StaffName(qtrc("InstrumentsXML", e.readElementText().toUtf8().data()), pos));
        } else if (tag == "shortName" || tag == "short-name") {     // "short-name" is obsolete
            int pos = e.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = shortNames.begin(); i != shortNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    shortNames.erase(i);
                    break;
                }
            }
            shortNames.append(StaffName(qtrc("InstrumentsXML", e.readElementText().toUtf8().data()), pos));
        } else if (tag == "trackName") {
            trackName = qtrc("InstrumentsXML", e.readElementText().toUtf8().data());
        } else if (tag == "description") {
            description = e.readElementText();
        } else if (tag == "extended") {
            extended = e.readInt();
        } else if (tag == "staves") {
            staffCount = e.readInt();
            bracketSpan[0] = staffCount;
//                  for (int i = 0; i < staves-1; ++i)
//                        barlineSpan[i] = true;
        } else if (tag == "clef") {             // sets both transposing and concert clef
            int idx = readStaffIdx(e);
            QString val(e.readElementText());
            bool ok;
            int i = val.toInt(&ok);
            ClefType ct = ok ? ClefType(i) : TConv::fromXml(val, ClefType::G);
            clefTypes[idx]._concertClef = ct;
            clefTypes[idx]._transposingClef = ct;
        } else if (tag == "concertClef") {
            int idx = readStaffIdx(e);
            QString val(e.readElementText());
            bool ok;
            int i = val.toInt(&ok);
            clefTypes[idx]._concertClef = ok ? ClefType(i) : TConv::fromXml(val, ClefType::G);
        } else if (tag == "transposingClef") {
            int idx = readStaffIdx(e);
            QString val(e.readElementText());
            bool ok;
            int i = val.toInt(&ok);
            clefTypes[idx]._transposingClef = ok ? ClefType(i) : TConv::fromXml(val, ClefType::G);
        } else if (tag == "stafflines") {
            int idx = readStaffIdx(e);
            staffLines[idx] = e.readInt();
        } else if (tag == "smallStaff") {
            int idx = readStaffIdx(e);
            smallStaff[idx] = e.readInt();
        } else if (tag == "bracket") {
            int idx = readStaffIdx(e);
            bracket[idx] = BracketType(e.readInt());
        } else if (tag == "bracketSpan") {
            int idx = readStaffIdx(e);
            bracketSpan[idx] = e.readInt();
        } else if (tag == "barlineSpan") {
            int idx = readStaffIdx(e);
            int span = e.readInt();
            for (int i = 0; i < span - 1; ++i) {
                barlineSpan[idx + i] = true;
            }
        } else if (tag == "aPitchRange") {
            setPitchRange(e.readElementText(), &minPitchA, &maxPitchA);
        } else if (tag == "pPitchRange") {
            setPitchRange(e.readElementText(), &minPitchP, &maxPitchP);
        } else if (tag == "transposition") {      // obsolete
            int i = e.readInt();
            transpose.chromatic = i;
            transpose.diatonic = chromatic2diatonic(i);
        } else if (tag == "transposeChromatic") {
            transpose.chromatic = e.readInt();
        } else if (tag == "transposeDiatonic") {
            transpose.diatonic = e.readInt();
        } else if (tag == "dropdownName") {
            trait.type = traitTypeFromString(e.attribute("meaning"));
            QString traitName = qtrc("InstrumentsXML", e.readElementText().toUtf8().data());
            trait.isDefault = traitName.contains("*");
            trait.isHiddenOnScore = traitName.contains("(") && traitName.contains(")");
            trait.name = traitName.remove("*").remove("(").remove(")");
        } else if (tag == "StringData") {
            stringData.read(e);
        } else if (tag == "drumset") {
            useDrumset = e.readInt();
        } else if (tag == "Drum") {
            // if we see one of this tags, a custom drumset will
            // be created
            if (drumset == 0) {
                drumset = new Drumset(*smDrumset);
                drumset->clear();
            }
            drumset->load(e);
        } else if (tag == "MidiAction") {
            NamedEventList a;
            a.read(e);
            midiActions.append(a);
        } else if (tag == "Channel" || tag == "channel") {
            Channel a;
            a.read(e, nullptr);
            channel.append(a);
        } else if (tag == "Articulation") {
            MidiArticulation a;
            a.read(e);
            int n = articulation.size();
            int i;
            for (i = 0; i < n; ++i) {
                if (articulation[i].name == a.name) {
                    articulation[i] = a;
                    break;
                }
            }
            if (i == n) {
                articulation.append(a);
            }
        } else if (tag == "stafftype") {
            int staffIdx = readStaffIdx(e);
            QString xmlPresetName = e.attribute("staffTypePreset", "");
            QString stfGroup = e.readElementText();
            if (stfGroup == "percussion") {
                staffGroup = StaffGroup::PERCUSSION;
            } else if (stfGroup == "tablature") {
                staffGroup = StaffGroup::TAB;
            } else {
                staffGroup = StaffGroup::STANDARD;
            }
            staffTypePreset = 0;
            if (!xmlPresetName.isEmpty()) {
                staffTypePreset = StaffType::presetFromXmlName(xmlPresetName);
            }
            if (!staffTypePreset || staffTypePreset->group() != staffGroup) {
                staffTypePreset = StaffType::getDefaultPreset(staffGroup);
            }
            if (staffTypePreset) {
                staffLines[staffIdx] = staffTypePreset->lines();
            }
        } else if (tag == "init") {
            QString val(e.readElementText());
            InstrumentTemplate* ttt = searchTemplate(val);
            if (ttt) {
                init(*ttt);
            } else {
                qDebug("InstrumentTemplate:: init instrument <%s> not found", qPrintable(val));
            }
        } else if (tag == "musicXMLid") {
            musicXMLid = e.readElementText();
        } else if (tag == "family") {
            family = searchInstrumentFamily(e.readElementText());
        } else if (tag == "genre") {
            QString val(e.readElementText());
            linkGenre(val);
        } else if (tag == "singleNoteDynamics") {
            singleNoteDynamics = e.readBool();
        } else {
            e.unknown();
        }
    }
    if (channel.empty()) {
        Channel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(Channel::DEFAULT_NAME);
        a.setProgram(0);
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        channel.append(a);
    }

    if (trackName.isEmpty() && !longNames.isEmpty()) {
        trackName = longNames[0].name();
    }
    if (description.isEmpty() && !longNames.isEmpty()) {
        description = longNames[0].name();
    }
    if (id.isEmpty()) {
        id = trackName.toLower().replace(" ", "-");
    }

    if (staffCount == 0) {
        qDebug(" 2Instrument: staves == 0 <%s>", qPrintable(id));
    }
}

//---------------------------------------------------------
//   setPitchRange
//---------------------------------------------------------

void InstrumentTemplate::setPitchRange(const QString& s, char* a, char* b) const
{
    QStringList sl = s.split("-");
    if (sl.size() != 2) {
        *a = 0;
        *b = 127;
        return;
    }
    *a = sl[0].toInt();
    *b = sl[1].toInt();
}

//---------------------------------------------------------
//   clearInstrumentTemplates
//---------------------------------------------------------

void clearInstrumentTemplates()
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        g->clear();
    }
    qDeleteAll(instrumentGroups);
    instrumentGroups.clear();
    qDeleteAll(instrumentGenres);
    instrumentGenres.clear();
    qDeleteAll(instrumentFamilies);
    instrumentFamilies.clear();
    articulation.clear();
    instrumentOrders.clear();
}

//---------------------------------------------------------
//   loadInstrumentTemplates
//---------------------------------------------------------

bool loadInstrumentTemplates(const QString& instrTemplates)
{
    QFile qf(instrTemplates);
    if (!qf.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug("cannot load instrument templates at <%s>", qPrintable(instrTemplates));
        return false;
    }

    XmlReader e(&qf);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                const QStringRef& tag(e.name());
                if (tag == "instrument-group" || tag == "InstrumentGroup") {
                    QString idGroup(e.attribute("id"));
                    InstrumentGroup* group = searchInstrumentGroup(idGroup);
                    if (group == 0) {
                        group = new InstrumentGroup;
                        instrumentGroups.append(group);
                    }
                    group->read(e);
                } else if (tag == "Articulation") {
                    // read global articulation
                    QString name(e.attribute("name"));
                    MidiArticulation a = searchArticulation(name);
                    a.read(e);
                    articulation.append(a);
                } else if (tag == "Genre") {
                    QString idGenre(e.attribute("id"));
                    InstrumentGenre* genre = searchInstrumentGenre(idGenre);
                    if (!genre) {
                        genre = new InstrumentGenre;
                        instrumentGenres.append(genre);
                    }
                    genre->read(e);
                } else if (tag == "Family") {
                    QString idFamily(e.attribute("id"));
                    InstrumentFamily* fam = searchInstrumentFamily(idFamily);
                    if (!fam) {
                        fam = new InstrumentFamily;
                        instrumentFamilies.append(fam);
                    }
                    fam->read(e);
                } else if (tag == "Order") {
                    ScoreOrder order;
                    order.read(e);
                    instrumentOrders.append(order);
                } else {
                    e.unknown();
                }
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   searchTemplate
//---------------------------------------------------------

InstrumentTemplate* searchTemplate(const QString& name)
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            if (it->id == name) {
                return it;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   searchTemplateForMusicXMLid
//---------------------------------------------------------

InstrumentTemplate* searchTemplateForMusicXmlId(const QString& mxmlId)
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            if (it->musicXMLid == mxmlId) {
                return it;
            }
        }
    }
    return 0;
}

InstrumentTemplate* searchTemplateForInstrNameList(const QList<QString>& nameList)
{
    InstrumentTemplate* bestMatch = nullptr; // default if no matches
    int bestMatchStrength = 0; // higher for better matches
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            for (const QString& name : nameList) {
                if (name.isEmpty()) {
                    continue;
                }

                int matchStrength = 0
                                    + (4 * (it->trackName == name ? 1 : 0)) // most weight to track name since there are fewer duplicates
                                    + (2 * (it->longNames.contains(StaffName(name)) ? 1 : 0))
                                    + (1 * (it->shortNames.contains(StaffName(name)) ? 1 : 0)); // least weight to short name
                const int perfectMatchStrength = 7;
                Q_ASSERT(matchStrength <= perfectMatchStrength);
                if (matchStrength > bestMatchStrength) {
                    bestMatch = it;
                    bestMatchStrength = matchStrength;
                    if (bestMatchStrength == perfectMatchStrength) {
                        break; // stop looking for matches
                    }
                }
            }
        }
    }
    return bestMatch; // nullptr if no matches found
}

InstrumentTemplate* searchTemplateForMidiProgram(int midiProgram, const bool useDrumSet)
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            if (it->channel.empty() || it->useDrumset != useDrumSet) {
                continue;
            }

            if (it->channel[0].program() == midiProgram) {
                return it;
            }
        }
    }
    return 0;
}

InstrumentTemplate* guessTemplateByNameData(const QList<QString>& nameDataList)
{
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            for (const QString& name : nameDataList) {
                if (name.contains(it->trackName, Qt::CaseInsensitive)
                    || name.contains(it->longNames.value(0).name(), Qt::CaseInsensitive)
                    || name.contains(it->shortNames.value(0).name(), Qt::CaseInsensitive)) {
                    return it;
                }
            }
        }
    }

    for (const QString& name : nameDataList) {
        if (name.contains("drum", Qt::CaseInsensitive)) {
            return searchTemplate("drumset");
        }

        if (name.contains("piano", Qt::CaseInsensitive)) {
            return searchTemplate("piano");
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   searchTemplateIndexForTrackName
//---------------------------------------------------------

InstrumentIndex searchTemplateIndexForTrackName(const QString& trackName)
{
    int instIndex = 0;
    int grpIndex = 0;
    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            if (it->trackName == trackName) {
                return InstrumentIndex(grpIndex, instIndex, it);
            }
            ++instIndex;
        }
        ++grpIndex;
    }
    return InstrumentIndex(-1, -1, nullptr);
}

//---------------------------------------------------------
//   searchTemplateIndexForId
//---------------------------------------------------------

InstrumentIndex searchTemplateIndexForId(const QString& id)
{
    int instIndex = 0;
    int grpIndex = 0;
    for (InstrumentGroup* g : instrumentGroups) {
        for (InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->id == id) {
                return InstrumentIndex(grpIndex, instIndex, it);
            }
            ++instIndex;
        }
        ++grpIndex;
    }
    return InstrumentIndex(-1, instIndex, nullptr);
}

//---------------------------------------------------------
//   linkGenre
//      link the current instrument template to the genre list specified by "genre"
//      Each genre is a list of pointers to instrument templates
//      The list of genres is at application level
//---------------------------------------------------------

void InstrumentTemplate::linkGenre(const QString& genre)
{
    InstrumentGenre* ig = searchInstrumentGenre(genre);
    if (ig) {
        genres.append(ig);
    }
}

void InstrumentGenre::write(XmlWriter& xml) const
{
    xml.startObject(QString("Genre id=\"%1\"").arg(id));
    xml.tag("name", name);
    xml.endObject();
}

void InstrumentGenre::write1(XmlWriter& xml) const
{
    write(xml);
}

void InstrumentGenre::read(XmlReader& e)
{
    id = e.attribute("id");
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "name") {
            name = qtrc("InstrumentsXML", e.readElementText().toUtf8().data());
        } else {
            e.unknown();
        }
    }
}

void InstrumentFamily::write(XmlWriter& xml) const
{
    xml.startObject(QString("Family id=\"%1\"").arg(id));
    xml.tag("name", name);
    xml.endObject();
}

void InstrumentFamily::write1(XmlWriter& xml) const
{
    write(xml);
}

void InstrumentFamily::read(XmlReader& e)
{
    id = e.attribute("id");
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "name") {
            name = qtrc("InstrumentsXML", e.readElementText().toUtf8().data());
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList InstrumentTemplate::clefType(int staffIdx) const
{
    if (staffIdx < staffCount) {
        return clefTypes[staffIdx];
    }
    return clefTypes[0];
}

QString InstrumentTemplate::familyId() const
{
    return family ? family->id : QString();
}

bool InstrumentTemplate::containsGenre(const QString& genreId) const
{
    for (const InstrumentGenre* genre : genres) {
        if (genre->id == genreId) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   defaultClef
//    traverse the instrument list for first instrument
//    with midi patch 'program'. Return the default clef
//    for this instrument.
//---------------------------------------------------------

ClefType defaultClef(int program)
{
    if (program >= 24 && program < 32) {              // this are guitars
        return ClefType::G8_VB;
    } else if (program >= 32 && program < 40) {       // this is bass
        return ClefType::F8_VB;
    }

    for (InstrumentGroup* g : qAsConst(instrumentGroups)) {
        for (InstrumentTemplate* it : qAsConst(g->instrumentTemplates)) {
            if (it->channel[0].bank() == 0 && it->channel[0].program() == program) {
                return it->clefTypes[0]._concertClef;
            }
        }
    }
    return ClefType::G;
}
}
