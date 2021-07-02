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
#include <iostream>

#include "scoreorder.h"
#include "translation.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/bracketItem.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/undo.h"
#include "libmscore/xml.h"

using namespace mu;

namespace Ms {
static const QString SOLOISTS_ID("<soloists>");
static const QString UNSORTED_ID("<unsorted>");

//---------------------------------------------------------
//   readBoolAttribute
//---------------------------------------------------------

bool ScoreOrder::readBoolAttribute(Ms::XmlReader& reader, const char* name, bool defvalue)
{
    if (!reader.hasAttribute(name)) {
        return defvalue;
    }
    QString attr { reader.attribute(name) };
    if (attr.toLower() == "false") {
        return false;
    } else if (attr.toLower() == "true") {
        return true;
    }
    qDebug("invalid value \"%s\" for attribute \"%s\", using default \"%d\"", qPrintable(attr), qPrintable(name), defvalue);
    return defvalue;
}

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

void ScoreOrder::readInstrument(Ms::XmlReader& reader)
{
    QString instrumentId { reader.attribute("id") };
    if (!Ms::searchTemplate(instrumentId)) {
        qDebug("cannot find instrument templates for <%s>", qPrintable(instrumentId));
        reader.skipCurrentElement();
        return;
    }
    while (reader.readNextStartElement()) {
        if (reader.name() == "family") {
            InstrumentOverwrite io;
            io.id = reader.attribute("id");
            io.name = qtrc("OrderXML", reader.readElementText().toUtf8().data());
            instrumentMap.insert(instrumentId, io);
        } else {
            reader.unknown();
        }
    }
}

//---------------------------------------------------------
//   readSoloists
//---------------------------------------------------------

void ScoreOrder::readSoloists(Ms::XmlReader& reader, const QString section)
{
    reader.skipCurrentElement();
    if (hasGroup(SOLOISTS_ID)) {
        return;
    }
    ScoreGroup sg;
    sg.family = QString(SOLOISTS_ID);
    sg.section = section;
    groups << sg;
}

//---------------------------------------------------------
//   readSection
//---------------------------------------------------------

void ScoreOrder::readSection(Ms::XmlReader& reader)
{
    QString sectionId { reader.attribute("id") };
    bool showSystemMarkings = readBoolAttribute(reader, "showSystemMarkings", false);
    bool barLineSpan = readBoolAttribute(reader, "barLineSpan", true);
    bool thinBrackets = readBoolAttribute(reader, "thinBrackets", true);
    while (reader.readNextStartElement()) {
        if (reader.name() == "family") {
            ScoreGroup sg;
            sg.family = reader.readElementText().toUtf8().data();
            sg.section = sectionId;
            sg.bracket = true;
            sg.showSystemMarkings = showSystemMarkings;
            sg.barLineSpan = barLineSpan;
            sg.thinBracket = thinBrackets;
            groups << sg;
        } else if (reader.name() == "unsorted") {
            QString group { reader.attribute("group", QString("")) };

            if (hasGroup(UNSORTED_ID, group)) {
                reader.skipCurrentElement();
                return;
            }

            ScoreGroup sg;
            sg.family = QString(UNSORTED_ID);
            sg.section = sectionId;
            sg.unsorted = group;
            sg.bracket = false;
            sg.showSystemMarkings = readBoolAttribute(reader, "showSystemMarkings", false);
            sg.barLineSpan = readBoolAttribute(reader, "barLineSpan", true);
            sg.thinBracket = readBoolAttribute(reader, "thinBrackets", true);
            groups << sg;
            reader.skipCurrentElement();
        } else {
            reader.unknown();
        }
    }
}

//---------------------------------------------------------
//   hasGroup
//---------------------------------------------------------

bool ScoreOrder::hasGroup(const QString& id, const QString& group) const
{
    for (const ScoreGroup& sg: groups) {
        if ((sg.family == id) && (group == sg.unsorted)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   isValid
//---------------------------------------------------------

bool ScoreOrder::isValid() const
{
    return !id.isEmpty();
}

//---------------------------------------------------------
//   getFamilyName
//---------------------------------------------------------

QString ScoreOrder::getFamilyName(const InstrumentTemplate* instrTemplate, bool soloist) const
{
    if (!instrTemplate) {
        return QString("<unsorted>");
    }

    if (soloist) {
        return QString("<soloists>");
    } else if (instrumentMap.contains(instrTemplate->id)) {
        return instrumentMap[instrTemplate->id].id;
    } else if (instrTemplate->family) {
        return instrTemplate->family->id;
    }
    return QString("<unsorted>");
}

//---------------------------------------------------------
//   getGroup
//---------------------------------------------------------

ScoreGroup ScoreOrder::getGroup(const QString family, const QString instrumentGroup) const
{
    static const QString UNSORTED = QString("<unsorted>");

    ScoreGroup unsortedScoreGroup;
    for (const ScoreGroup& sg : groups) {
        if ((sg.family == UNSORTED) && sg.unsorted.isEmpty()) {
            unsortedScoreGroup = sg;
            break;
        }
    }

    if (family.isEmpty()) {
        return unsortedScoreGroup;
    }

    for (const ScoreGroup& sg : groups) {
        if (sg.family == family) {
            return sg;
        }
        if ((sg.family == UNSORTED) && (sg.unsorted == instrumentGroup)) {
            unsortedScoreGroup = sg;
        }
    }
    return unsortedScoreGroup;
}

//---------------------------------------------------------
//   setBracketsAndBarlines
//---------------------------------------------------------

void ScoreOrder::setBracketsAndBarlines(Score* score)
{
    if (groups.size() <= 0) {
        return;
    }

    bool prvThnBracket { false };
    bool prvBarLineSpan { false };
    QString prvSection { "" };
    int prvInstrument { 0 };
    Staff* prvStaff { nullptr };

    Staff* thkBracketStaff { nullptr };
    Staff* thnBracketStaff { nullptr };
    int thkBracketSpan { 0 };
    int thnBracketSpan { 0 };

    for (Part* part : score->parts()) {
        InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->getId());
        if (!ii.instrTemplate) {
            continue;
        }

        QString family { getFamilyName(ii.instrTemplate, part->soloist()) };
        const ScoreGroup sg = getGroup(family, instrumentGroups[ii.groupIndex]->id);

        int staffIdx { 0 };
        bool blockThinBracket { false };
        for (Staff* staff : *part->staves()) {
            for (BracketItem* bi : staff->brackets()) {
                score->undo(new RemoveBracket(staff, bi->column(), bi->bracketType(), bi->bracketSpan()));
            }
            staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, 0);

            if (prvSection.isEmpty() || (sg.section != prvSection)) {
                if (thkBracketStaff && (thkBracketSpan > 1)) {
                    score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
                }
                if (sg.bracket && !staffIdx) {
                    thkBracketStaff = sg.bracket ? staff : nullptr;
                    thkBracketSpan  = 0;
                }
            }
            if (sg.bracket && !staffIdx) {
                thkBracketSpan += part->nstaves();
            }

            if (!staffIdx || (ii.instrIndex != prvInstrument)) {
                if (thnBracketStaff && (thnBracketSpan > 1)) {
                    score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
                }
                if (ii.instrIndex != prvInstrument) {
                    thnBracketStaff = (sg.thinBracket && !blockThinBracket) ? staff : nullptr;
                    thnBracketSpan  = 0;
                }
            }

            if (ii.instrTemplate->nstaves() > 1) {
                blockThinBracket = true;
                if (ii.instrTemplate->bracket[staffIdx] != BracketType::NO_BRACKET) {
                    score->undoAddBracket(staff, 2, ii.instrTemplate->bracket[staffIdx], ii.instrTemplate->bracketSpan[staffIdx]);
                }
                staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, ii.instrTemplate->barlineSpan[staffIdx]);
                if (staffIdx < ii.instrTemplate->nstaves()) {
                    ++staffIdx;
                }
                prvStaff = nullptr;
            } else {
                if (sg.thinBracket && !staffIdx) {
                    thnBracketSpan += part->nstaves();
                }
                if (prvStaff) {
                    prvStaff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,
                                                 (prvBarLineSpan && (!prvSection.isEmpty() && (sg.section == prvSection))));
                }
                prvStaff = staff;
                ++staffIdx;
            }
            prvSection = sg.section;
            prvBarLineSpan = sg.barLineSpan;
            prvThnBracket = sg.thinBracket;
        }

        prvInstrument = ii.instrIndex;
    }

    if (thkBracketStaff && (thkBracketSpan > 1)) {
        score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
    }
    if (thnBracketStaff && (thnBracketSpan > 1) && prvThnBracket) {
        score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreOrder::read(Ms::XmlReader& reader)
{
    id = reader.attribute("id");
    const QString sectionId { "" };
    while (reader.readNextStartElement()) {
        if (reader.name() == "name") {
            name = qtrc("OrderXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "section") {
            readSection(reader);
        } else if (reader.name() == "instrument") {
            readInstrument(reader);
        } else if (reader.name() == "family") {
            ScoreGroup sg;
            sg.family = reader.readElementText().toUtf8().data();
            sg.section = sectionId;
            sg.bracket = false;
            sg.showSystemMarkings = false;
            sg.barLineSpan = false;
            sg.thinBracket = false;
            groups << sg;
        } else if (reader.name() == "soloists") {
            readSoloists(reader, sectionId);
        } else if (reader.name() == "unsorted") {
            QString group { reader.attribute("group", QString("")) };

            if (hasGroup(UNSORTED_ID, group)) {
                reader.skipCurrentElement();
                return;
            }

            ScoreGroup sg;
            sg.family = QString(UNSORTED_ID);
            sg.section = sectionId;
            sg.unsorted = group;
            sg.bracket = true;
            sg.showSystemMarkings = false;
            sg.barLineSpan = false;
            sg.thinBracket = false;
            groups << sg;
            reader.skipCurrentElement();
        } else {
            reader.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ScoreOrder::write(Ms::XmlWriter& xml) const
{
    if (!isValid()) {
        return;
    }

    xml.stag(QString("Order id=\"%1\"").arg(id));
    xml.tag("name", name);

    QMapIterator<QString, InstrumentOverwrite> i(instrumentMap);
    while (i.hasNext()) {
        i.next();
        xml.stag(QString("instrument id=\"%1\"").arg(i.key()));
        xml.tag(QString("family id=\"%1\"").arg(i.value().id), i.value().name);
        xml.etag();
    }

    QString section { "" };
    for (const ScoreGroup& sg : groups) {
        if (sg.section != section) {
            if (!section.isEmpty()) {
                xml.etag();
            }
            if (!sg.section.isEmpty()) {
                xml.stag(QString("section id=\"%1\" brackets=\"%2\" showSystemMarkings=\"%3\" barLineSpan=\"%4\" thinBrackets=\"%5\"")
                         .arg(sg.section,
                              sg.bracket ? "true" : "false",
                              sg.showSystemMarkings ? "true" : "false",
                              sg.barLineSpan ? "true" : "false",
                              sg.thinBracket ? "true" : "false"));
            }
            section = sg.section;
        }
        if (sg.family == SOLOISTS_ID) {
            xml.tagE("soloists");
        } else if (sg.unsorted.isNull()) {
            xml.tag("family", sg.family);
        } else if (sg.unsorted.isEmpty()) {
            xml.tagE("unsorted");
        } else {
            xml.tagE(QString("unsorted group=\"%1\"").arg(sg.unsorted));
        }
    }
    if (!section.isEmpty()) {
        xml.etag();
    }
    xml.etag();
}

//---------------------------------------------------------
//   updateInstruments
//---------------------------------------------------------

void ScoreOrder::updateInstruments(const Score* score)
{
    for (Part* part : score->parts()) {
        InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->getId());
        if (!ii.instrTemplate || !ii.instrTemplate->family) {
            continue;
        }

        InstrumentFamily* family = ii.instrTemplate->family;
        InstrumentOverwrite io;
        io.id = family->id;
        io.name = family->name;
        instrumentMap.insert(ii.instrTemplate->id, io);
    }
}
}
