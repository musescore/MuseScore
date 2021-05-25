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

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/bracketItem.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/undo.h"
#include "libmscore/xml.h"

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
    } else {
        qDebug("invalid value \"%s\" for attribute \"%s\", using default \"%d\"", qPrintable(attr), qPrintable(name), defvalue);
        return defvalue;
    }
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
            io.name = qApp->translate("OrderXML", reader.readElementText().toUtf8().data());
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
//   readUnsorted
//---------------------------------------------------------

void ScoreOrder::readUnsorted(Ms::XmlReader& reader, const QString section, bool inSection)
{
    QString group { reader.attribute("group", QString("")) };
    reader.skipCurrentElement();

    if (hasGroup(UNSORTED_ID, group)) {
        return;
    }

    ScoreGroup sg;
    sg.family = QString(UNSORTED_ID);
    sg.section = section;
    sg.unsorted = group;
    sg.bracket            = !inSection ? true : false;
    sg.showSystemMarkings = !inSection ? false : readBoolAttribute(reader, "showSystemMarkings", false);
    sg.barLineSpan        = !inSection ? false : readBoolAttribute(reader, "barLineSpan",        true);
    sg.thinBracket        = !inSection ? false : readBoolAttribute(reader, "thinBrackets",       true);
    groups << sg;
}

//---------------------------------------------------------
//   readFamily
//---------------------------------------------------------

void ScoreOrder::readFamily(Ms::XmlReader& reader, const QString section, bool inSection)
{
    const QString id { reader.readElementText().toUtf8().data() };

    ScoreGroup sg;
    sg.family = id;
    sg.section = section;
    sg.bracket            = !inSection ? false : true;
    sg.showSystemMarkings = !inSection ? false : readBoolAttribute(reader, "showSystemMarkings", false);
    sg.barLineSpan        = !inSection ? false : readBoolAttribute(reader, "barLineSpan",        true);
    sg.thinBracket        = !inSection ? false : readBoolAttribute(reader, "thinBrackets",       true);
    groups << sg;
}

//---------------------------------------------------------
//   readSection
//---------------------------------------------------------

void ScoreOrder::readSection(Ms::XmlReader& reader)
{
    QString id { reader.attribute("id") };
    while (reader.readNextStartElement()) {
        if (reader.name() == "family") {
            readFamily(reader, id, true);
        } else if (reader.name() == "unsorted") {
            readUnsorted(reader, id, true);
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
//   read
//---------------------------------------------------------

void ScoreOrder::read(Ms::XmlReader& reader)
{
    id = reader.attribute("id");
    const QString id { "" };
    while (reader.readNextStartElement()) {
        if (reader.name() == "name") {
            name = qApp->translate("OrderXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "section") {
            readSection(reader);
        } else if (reader.name() == "instrument") {
            readInstrument(reader);
        } else if (reader.name() == "family") {
            readFamily(reader, id, false);
        } else if (reader.name() == "soloists") {
            readSoloists(reader, id);
        } else if (reader.name() == "unsorted") {
            readUnsorted(reader, id, false);
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
