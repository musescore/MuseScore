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

#include "compat/midi/compatmidirender.h"
#include "dom/factory.h"
#include "dom/instrtemplate.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/text.h"
#include "editing/editpart.h"
#include "editing/editsystemlocks.h"
#include "types/typesconv.h"

// api
#include "apistructs.h"
#include "cursor.h"
#include "elements.h"
#include "apitypes.h"

using namespace mu::engraving::apiv1;

Cursor* Score::newCursor()
{
    return new Cursor(score());
}

void Score::addText(const QString& type, const QString& txt)
{
    static const QMetaEnum meta = QMetaEnum::fromType<enums::TextStyleType>();

    mu::engraving::TextStyleType tid;
    const std::string key = type.toStdString();
    bool ok = false;
    int val = meta.keyToValue(key.c_str(), &ok);
    if (ok) {
        tid = static_cast<mu::engraving::TextStyleType>(val);
    } else {
        LOGE() << "Please use engraving::TextStyleType enum, the use of Xml tags is deprecated.";
        AsciiStringView t(key);
        tid = mu::engraving::TConv::fromXml(t, mu::engraving::TextStyleType::DEFAULT);
    }

    mu::engraving::MeasureBase* mb = score()->first();
    if (!mb || !mb->isVBox()) {
        score()->insertBox(ElementType::VBOX, mb);
        mb = score()->first();
    }

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

/** APIDOC
* Replaces the instrument for a given part with a new instrument.
* This changes the instrument definition including its name, clef, and sound.
* @method
* @param {Part} part - The Part object whose instrument should be replaced.
* @param {string} instrumentId - ID of the new instrument from instruments.xml.
*/
void Score::replaceInstrument(apiv1::Part* part, const QString& instrumentId)
{
    if (!part) {
        LOGW("replaceInstrument: part is null");
        return;
    }

    const InstrumentTemplate* t = searchTemplate(instrumentId);
    if (!t) {
        LOGW("replaceInstrument: <%s> not found", qPrintable(instrumentId));
        return;
    }

    mu::engraving::Instrument newInstrument = mu::engraving::Instrument::fromTemplate(t);
    mu::engraving::EditPart::replacePartInstrument(score(), part->part(), newInstrument);
}

void Score::setPartVisible(apiv1::Part* part, bool visible)
{
    if (!part) {
        LOGW("setPartVisible: part is null");
        return;
    }

    mu::engraving::EditPart::setPartVisible(score(), part->part(), visible);
}

void Score::setStaffVisible(apiv1::Staff* staff, bool visible)
{
    if (!staff) {
        LOGW("setStaffVisible: staff is null");
        return;
    }

    mu::engraving::EditPart::setStaffVisible(score(), staff->staff(), visible);
}

void Score::setPartSharpFlat(apiv1::Part* part, int sharpFlat)
{
    if (!part) {
        LOGW("setPartSharpFlat: part is null");
        return;
    }

    mu::engraving::EditPart::setPartSharpFlat(score(), part->part(), mu::engraving::PreferSharpFlat(sharpFlat));
}

void Score::setInstrumentName(apiv1::Part* part, const QString& name)
{
    if (!part) {
        LOGW("setInstrumentName: part is null");
        return;
    }

    mu::engraving::EditPart::setInstrumentName(score(), part->part(), mu::engraving::Fraction(0, 1), name);
}

void Score::setInstrumentAbbreviature(apiv1::Part* part, const QString& abbreviature)
{
    if (!part) {
        LOGW("setInstrumentAbbreviature: part is null");
        return;
    }

    mu::engraving::EditPart::setInstrumentAbbreviature(score(), part->part(), mu::engraving::Fraction(0, 1), abbreviature);
}

void Score::setStaffType(apiv1::Staff* staff, int staffTypeId)
{
    if (!staff) {
        LOGW("setStaffType: staff is null");
        return;
    }

    mu::engraving::EditPart::setStaffType(score(), staff->staff(), mu::engraving::StaffTypes(staffTypeId));
}

void Score::removeParts(QList<apiv1::Part*> parts)
{
    std::vector<mu::engraving::Part*> domParts;
    for (apiv1::Part* p : parts) {
        if (!p) {
            LOGW("removeParts: null part in list");
            continue;
        }
        domParts.push_back(p->part());
    }

    mu::engraving::EditPart::removeParts(score(), domParts);
}

void Score::removeStaves(QList<apiv1::Staff*> staves)
{
    std::vector<mu::engraving::Staff*> domStaves;
    for (apiv1::Staff* s : staves) {
        if (!s) {
            LOGW("removeStaves: null staff in list");
            continue;
        }
        domStaves.push_back(s->staff());
    }

    mu::engraving::EditPart::removeStaves(score(), domStaves);
}

void Score::moveParts(QList<apiv1::Part*> sourceParts, apiv1::Part* destinationPart, int insertMode)
{
    if (!destinationPart) {
        LOGW("moveParts: destinationPart is null");
        return;
    }

    std::vector<mu::engraving::Part*> domParts;
    for (apiv1::Part* p : sourceParts) {
        if (!p) {
            LOGW("moveParts: null part in list");
            continue;
        }
        domParts.push_back(p->part());
    }

    mu::engraving::EditPart::moveParts(score(), domParts, destinationPart->part(), insertMode == 1);
}

void Score::moveStaves(QList<apiv1::Staff*> sourceStaves, apiv1::Staff* destinationStaff, int insertMode)
{
    if (!destinationStaff) {
        LOGW("moveStaves: destinationStaff is null");
        return;
    }

    std::vector<mu::engraving::Staff*> domStaves;
    for (apiv1::Staff* s : sourceStaves) {
        if (!s) {
            LOGW("moveStaves: null staff in list");
            continue;
        }
        domStaves.push_back(s->staff());
    }

    mu::engraving::EditPart::moveStaves(score(), domStaves, destinationStaff->staff(), insertMode == 1);
}

void Score::addSystemObjects(QList<apiv1::Staff*> staves)
{
    std::vector<mu::engraving::Staff*> domStaves;
    for (apiv1::Staff* s : staves) {
        if (!s) {
            LOGW("addSystemObjects: null staff in list");
            continue;
        }
        domStaves.push_back(s->staff());
    }

    mu::engraving::EditPart::addSystemObjects(score(), domStaves);
}

void Score::removeSystemObjects(QList<apiv1::Staff*> staves)
{
    std::vector<mu::engraving::Staff*> domStaves;
    for (apiv1::Staff* s : staves) {
        if (!s) {
            LOGW("removeSystemObjects: null staff in list");
            continue;
        }
        domStaves.push_back(s->staff());
    }

    mu::engraving::EditPart::removeSystemObjects(score(), domStaves);
}

void Score::moveSystemObjects(apiv1::Staff* sourceStaff, apiv1::Staff* destinationStaff)
{
    if (!sourceStaff) {
        LOGW("moveSystemObjects: sourceStaff is null");
        return;
    }
    if (!destinationStaff) {
        LOGW("moveSystemObjects: destinationStaff is null");
        return;
    }

    mu::engraving::EditPart::moveSystemObjects(score(), sourceStaff->staff(), destinationStaff->staff());
}

void Score::setStaffConfig(apiv1::Staff* staff, bool visible, double userDistance,
                           bool cutaway, bool hideSystemBarLine, int mergeMatchingRests,
                           bool reflectTransposition, int staffTypeId)
{
    if (!staff) {
        LOGW("setStaffConfig: staff is null");
        return;
    }

    mu::engraving::EditPart::setStaffConfig(score(), staff->staff(), visible, userDistance,
                                            cutaway, hideSystemBarLine, mergeMatchingRests,
                                            reflectTransposition, mu::engraving::StaffTypes(staffTypeId));
}

//---------------------------------------------------------
//   Score::firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(int segmentType)
{
    return wrap<Segment>(score()->firstSegment(engraving::SegmentType(segmentType)), Ownership::SCORE);
}

Measure* Score::tick2measure(Fraction* f)
{
    const mu::engraving::Fraction tick = f->fraction();
    if (!tick.isValid() || tick.negative()) {
        return nullptr;
    }
    return wrap<Measure>(score()->tick2measure(tick));
}

Segment* Score::findSegmentAtTick(int segmentTypes, Fraction* f)
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

QQmlListProperty<Staff> Score::staves() const
{
    return wrapContainerProperty<Staff>(this, score()->staves());
}

QQmlListProperty<Part> Score::parts() const
{
    return wrapContainerProperty<Part>(this, score()->parts());
}

QQmlListProperty<Excerpt> Score::excerpts() const
{
    return wrapExcerptsContainerProperty<Excerpt>(this, score()->masterScore()->excerpts());
}

QQmlListProperty<Page> Score::pages() const
{
    return wrapContainerProperty<Page>(this, score()->pages());
}

QQmlListProperty<System> Score::systems() const
{
    return wrapContainerProperty<System>(this, score()->systems());
}

bool Score::hasLyrics() const
{
    return score()->hasLyrics();
}

int Score::lyricCount() const
{
    return score()->lyricCount();
}

QQmlListProperty<Lyrics> Score::lyrics() const
{
    static std::vector<engraving::Lyrics*> list;
    list = score()->lyrics();
    return wrapContainerProperty<Lyrics>(this, list);
}

QString Score::extractLyrics() const
{
    return score()->extractLyrics();
}

QQmlListProperty<Spanner> Score::spanners()
{
    static std::vector<mu::engraving::Spanner*> spannerList;
    spannerList = score()->spannerList();
    return wrapContainerProperty<Spanner>(this, spannerList);
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

    notation()->notationChanged().send(muse::RectF());
}

void Score::doLayout(Fraction* startTick, Fraction* endTick)
{
    score()->doLayoutRange(startTick->fraction(), endTick->fraction());
}

void Score::addRemoveSystemLocks(int interval, bool lock)
{
    EditSystemLocks::addRemoveSystemLocks(score(), interval, lock);
}

void Score::makeIntoSystem(apiv1::MeasureBase* first, apiv1::MeasureBase* last)
{
    EditSystemLocks::makeIntoSystem(score(), first->measureBase(), last->measureBase());
}

void Score::showElementInScore(apiv1::EngravingItem* wrappedElement, int staffIdx)
{
    if (!wrappedElement->element()) {
        return;
    }
    notation()->interaction()->showItem(wrappedElement->element(), staffIdx);
}
