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

#include "score.h"
#include "cursor.h"
#include "elements.h"

#include "libmscore/factory.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/masterscore.h"
#include "libmscore/segment.h"
#include "libmscore/text.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   Score::newCursor
//---------------------------------------------------------

Cursor* Score::newCursor()
{
    return new Cursor(score());
}

//---------------------------------------------------------
//   Score::addText
///   \brief Adds a header text to the score.
///   \param type One of the following values:
///   - "title"
///   - "subtitle"
///   - "composer"
///   - "lyricist"
///   - Any other value corresponds to default text style.
///   \param txt Text to be added.
//---------------------------------------------------------

void Score::addText(const QString& type, const QString& txt)
{
    MeasureBase* measure = score()->first();
    if (!measure || !measure->isVBox()) {
        score()->insertMeasure(ElementType::VBOX, measure);
        measure = score()->first();
    }
    Ms::TextStyleType tid = Ms::TextStyleType::DEFAULT;
    if (type == "title") {
        tid = Ms::TextStyleType::TITLE;
    } else if (type == "subtitle") {
        tid = Ms::TextStyleType::SUBTITLE;
    } else if (type == "composer") {
        tid = Ms::TextStyleType::COMPOSER;
    } else if (type == "lyricist") {
        tid = Ms::TextStyleType::POET;
    }

    Ms::Text* text = mu::engraving::Factory::createText(measure, tid);
    text->setParent(measure);
    text->setXmlText(txt);
    score()->undoAddElement(text);
}

//---------------------------------------------------------
//   defaultInstrTemplate
//---------------------------------------------------------

static const Ms::InstrumentTemplate* defaultInstrTemplate()
{
    static Ms::InstrumentTemplate defaultInstrument;
    if (defaultInstrument.channel.empty()) {
        Ms::Channel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(Ms::Channel::DEFAULT_NAME);
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        defaultInstrument.channel.append(a);
    }
    return &defaultInstrument;
}

//---------------------------------------------------------
//   instrTemplateFromName
//---------------------------------------------------------

const InstrumentTemplate* Score::instrTemplateFromName(const QString& name)
{
    const InstrumentTemplate* t = searchTemplate(name);
    if (!t) {
        qWarning("<%s> not found", qPrintable(name));
        t = defaultInstrTemplate();
    }
    return t;
}

//---------------------------------------------------------
//   Score::appendPart
//---------------------------------------------------------

void Score::appendPart(const QString& instrumentId)
{
    const InstrumentTemplate* t = searchTemplate(instrumentId);

    if (!t) {
        qWarning("appendPart: <%s> not found", qPrintable(instrumentId));
        t = defaultInstrTemplate();
    }

    score()->appendPart(t);
}

//---------------------------------------------------------
//   Score::appendPartByMusicXmlId
//---------------------------------------------------------

void Score::appendPartByMusicXmlId(const QString& instrumentMusicXmlId)
{
    const InstrumentTemplate* t = searchTemplateForMusicXmlId(instrumentMusicXmlId);

    if (!t) {
        qWarning("appendPart: <%s> not found", qPrintable(instrumentMusicXmlId));
        t = defaultInstrTemplate();
    }

    score()->appendPart(t);
}

//---------------------------------------------------------
//   Score::firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment()
{
    return wrap<Segment>(score()->firstSegment(Ms::SegmentType::All), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::lastSegment
//---------------------------------------------------------

Segment* Score::lastSegment()
{
    return wrap<Segment>(score()->lastSegment(), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure()
{
    return wrap<Measure>(score()->firstMeasure(), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::firstMeasureMM
//---------------------------------------------------------

Measure* Score::firstMeasureMM()
{
    return wrap<Measure>(score()->firstMeasureMM(), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure()
{
    return wrap<Measure>(score()->lastMeasure(), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::firstMeasureMM
//---------------------------------------------------------

Measure* Score::lastMeasureMM()
{
    return wrap<Measure>(score()->lastMeasureMM(), Ownership::SCORE);
}

//---------------------------------------------------------
//   Score::staves
//---------------------------------------------------------

QQmlListProperty<Staff> Score::staves()
{
    return wrapContainerProperty<Staff>(this, score()->staves());
}

//---------------------------------------------------------
//   Score::startCmd
//---------------------------------------------------------

void Score::startCmd()
{
    score()->startCmd();
}
}
}
