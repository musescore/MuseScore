/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/compat/midi/compatmidirender.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/text.h"

#include "engraving/types/typesconv.h"

// api
#include "apistructs.h"
#include "cursor.h"
#include "elements.h"

using namespace mu::engraving::apiv1;

Cursor* Score::newCursor()
{
    return new Cursor(score());
}

//---------------------------------------------------------
//   Score::addText
///   \brief Adds a header text to the score, and a title frame if needed.
///   \param type The text style for the text, for example:
///   - "title"
///   - "subtitle"
///   - "composer"
///   - "lyricist"
///   \param txt Text to be added.
//---------------------------------------------------------

void Score::addText(const QString& type, const QString& txt)
{
    mu::engraving::MeasureBase* mb = score()->first();
    if (!mb || !mb->isVBox()) {
        score()->insertBox(ElementType::VBOX, mb);
        mb = score()->first();
    }
    AsciiStringView t(String::fromQString(type).toStdString());
    mu::engraving::TextStyleType tid = mu::engraving::TConv::fromXml(t, mu::engraving::TextStyleType::DEFAULT);
    mu::engraving::Text* text = mu::engraving::Factory::createText(mb, tid);
    text->setParent(mb);
    text->setXmlText(txt);
    score()->undoAddElement(text);
}

//---------------------------------------------------------
//   defaultInstrTemplate
//---------------------------------------------------------

static const mu::engraving::InstrumentTemplate* defaultInstrTemplate()
{
    static mu::engraving::InstrumentTemplate defaultInstrument;
    if (defaultInstrument.channel.empty()) {
        mu::engraving::InstrChannel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(muse::String::fromUtf8(mu::engraving::InstrChannel::DEFAULT_NAME));
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        defaultInstrument.channel.push_back(a);
    }
    return &defaultInstrument;
}

//---------------------------------------------------------
//   instrTemplateFromName
//---------------------------------------------------------

const mu::engraving::InstrumentTemplate* Score::instrTemplateFromName(const QString& name)
{
    const InstrumentTemplate* t = searchTemplate(name);
    if (!t) {
        LOGW("<%s> not found", qPrintable(name));
        t = defaultInstrTemplate();
    }
    return t;
}

mu::notation::INotationPtr Score::notation() const
{
    return context()->currentNotation();
}

mu::notation::INotationUndoStackPtr Score::undoStack() const
{
    return notation() ? notation()->undoStack() : nullptr;
}

//---------------------------------------------------------
//   Score::appendPart
//---------------------------------------------------------

void Score::appendPart(const QString& instrumentId)
{
    const InstrumentTemplate* t = searchTemplate(instrumentId);

    if (!t) {
        LOGW("appendPart: <%s> not found", qPrintable(instrumentId));
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
        LOGW("appendPart: <%s> not found", qPrintable(instrumentMusicXmlId));
        t = defaultInstrTemplate();
    }

    score()->appendPart(t);
}

//---------------------------------------------------------
//   Score::firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(int segmentType)
{
    return wrap<Segment>(score()->firstSegment(engraving::SegmentType(segmentType)), Ownership::SCORE);
}

Measure* Score::tick2measure(FractionWrapper* f)
{
    const mu::engraving::Fraction tick = f->fraction();
    if (!tick.isValid() || tick.negative()) {
        return nullptr;
    }
    return wrap<Measure>(score()->tick2measure(tick));
}

Segment* Score::findSegmentAtTick(int segmentTypes, FractionWrapper* f)
{
    const mu::engraving::Fraction tick = f->fraction();
    if (!tick.isValid() || tick.negative()) {
        return nullptr;
    }
    mu::engraving::Measure* measure = score()->tick2measure(tick);
    mu::engraving::Segment* segment = measure->findSegment(engraving::SegmentType(segmentTypes), tick);
    return segment ? wrap<Segment>(segment, Ownership::SCORE) : nullptr;
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

QString Score::name() const
{
    return score()->masterScore()->name();
}

void Score::setName(const QString& /*name*/)
{
    NOT_IMPLEMENTED;
}

void Score::createPlayEvents()
{
    mu::engraving::CompatMidiRender::createPlayEvents(score());
}

//---------------------------------------------------------
//   Score::staves
//---------------------------------------------------------

QQmlListProperty<Staff> Score::staves()
{
    return wrapContainerProperty<Staff>(this, score()->staves());
}

//---------------------------------------------------------
//   Score::pages
//---------------------------------------------------------

QQmlListProperty<Page> Score::pages()
{
    return wrapContainerProperty<Page>(this, score()->pages());
}

//---------------------------------------------------------
//   Score::systems
//---------------------------------------------------------

QQmlListProperty<System> Score::systems()
{
    return wrapContainerProperty<System>(this, score()->systems());
}

//---------------------------------------------------------
//   Score::startCmd
//---------------------------------------------------------

void Score::startCmd(const QString& qActionName)
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    muse::TranslatableString actionName = qActionName.isEmpty()
                                          ? TranslatableString("undoableAction", "Plugin edit")
                                          : TranslatableString::untranslatable(qActionName);

    undoStack()->prepareChanges(actionName);
    // Lock the undo stack, so that all changes made by the plugin,
    // including PluginAPI::cmd(), are committed as a single command.
    undoStack()->lock();
}

void Score::endCmd(bool rollback)
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    undoStack()->unlock();

    if (rollback) {
        undoStack()->rollbackChanges();
    } else {
        undoStack()->commitChanges();
    }

    notation()->notationChanged().notify();
}

void Score::doLayout(FractionWrapper* startTick, FractionWrapper* endTick)
{
    score()->doLayoutRange(startTick->fraction(), endTick->fraction());
}

void Score::makeIntoSystem(apiv1::MeasureBase* first, apiv1::MeasureBase* last)
{
    score()->makeIntoSystem(first->measureBase(), last->measureBase());
}

void Score::showElementInScore(apiv1::EngravingItem* wrappedElement, int staffIdx)
{
    if (!wrappedElement->element()) {
        return;
    }
    notation()->interaction()->showItem(wrappedElement->element(), staffIdx);
}
