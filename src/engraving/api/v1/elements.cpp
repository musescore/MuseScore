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

#include "elements.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurenumber.h"
#include "engraving/dom/mmrestrange.h"
#include "engraving/dom/property.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spacer.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/undo.h"

// api
#include "apistructs.h"
#include "part.h"

#include "log.h"

using namespace mu::engraving::apiv1;

//---------------------------------------------------------
//   EngravingItem::setOffsetX
//---------------------------------------------------------

void EngravingItem::setOffsetX(qreal offX)
{
    const qreal offY = element()->offset().y() / element()->spatium();
    set(mu::engraving::Pid::OFFSET, QPointF(offX, offY));
}

//---------------------------------------------------------
//   EngravingItem::setOffsetY
//---------------------------------------------------------

void EngravingItem::setOffsetY(qreal offY)
{
    const qreal offX = element()->offset().x() / element()->spatium();
    set(mu::engraving::Pid::OFFSET, QPointF(offX, offY));
}

static QRectF scaleRect(const mu::engraving::RectF& rect, double spatium)
{
    return QRectF(rect.x() / spatium, rect.y() / spatium, rect.width() / spatium, rect.height() / spatium);
}

//---------------------------------------------------------
//   EngravingItem::bbox
//   return the element bbox in spatium units, rather than in raster units as stored internally
//---------------------------------------------------------

QRectF EngravingItem::bbox() const
{
    return scaleRect(element()->ldata()->bbox(), element()->spatium());
}

bool EngravingItem::up() const
{
    if (element()->isChordRest()) {
        return toChordRest(element())->ldata()->up;
    } else if (element()->isStem()) {
        return toStem(element())->up();
    } else if (element()->isSlur()) {
        return toSlur(element())->up();
    } else if (element()->isTie()) {
        return toTie(element())->up();
    } else if (element()->isSlurTieSegment()) {
        return toSlurTieSegment(element())->slurTie()->up();
    } else if (element()->isArticulation()) {
        return toArticulation(element())->ldata()->up;
    } else if (element()->isGuitarBend()) {
        return toGuitarBend(element())->ldata()->up();
    } else if (element()->isGuitarBendSegment()) {
        return toGuitarBendSegment(element())->guitarBend()->ldata()->up();
    } else if (element()->isBeam()) {
        return toBeam(element())->up();
    } else if (element()->isTuplet()) {
        return toTuplet(element())->isUp();
    } else if (element()->type() == mu::engraving::ElementType::TREMOLO_TWOCHORD) {
        return item_cast<const TremoloTwoChord*>(element())->up();
    }
    return false;
}

FractionWrapper* EngravingItem::tick() const
{
    return wrap(element()->tick());
}

//---------------------------------------------------------
//   ChordRest::actualBeamMode
//---------------------------------------------------------

int ChordRest::actualBeamMode(bool beamRests)
{
    if (chordRest()->isRest() && !beamRests && chordRest()->beamMode() == mu::engraving::BeamMode::AUTO) {
        return int(mu::engraving::BeamMode::NONE);
    }
    mu::engraving::ChordRest* prev = nullptr;
    if (!(chordRest()->isChord() && toChord(chordRest())->isGrace())) {
        mu::engraving::Segment* seg = chordRest()->segment();
        mu::engraving::Measure* m = seg->measure();
        while (seg && seg->measure()->system() == m->system()) {
            seg = seg->prev1(mu::engraving::SegmentType::ChordRest);
            if (seg->element(chordRest()->track())) {
                prev = toChordRest(seg->element(chordRest()->track()));
                break;
            }
        }
    }
    return int(mu::engraving::Groups::endBeam(chordRest(), prev));
}

//---------------------------------------------------------
//   Segment::elementAt
//---------------------------------------------------------

EngravingItem* Segment::elementAt(int track)
{
    mu::engraving::EngravingItem* el = segment()->elementAt(track);
    if (!el) {
        return nullptr;
    }
    return wrap(el, Ownership::SCORE);
}

FractionWrapper* Segment::fraction() const
{
    return wrap(segment()->tick());
}

//---------------------------------------------------------
//   Note::setTpc
//---------------------------------------------------------

void Note::setTpc(int val)
{
    if (!tpcIsValid(val)) {
        LOGW("PluginAPI::Note::setTpc: invalid tpc: %d", val);
        return;
    }

    if (note()->concertPitch()) {
        set(Pid::TPC1, val);
    } else {
        set(Pid::TPC2, val);
    }
}

//---------------------------------------------------------
//   Note::isChildAllowed
///   Check if element type can be a child of note.
///   \since MuseScore 3.3.3
//---------------------------------------------------------

bool Note::isChildAllowed(mu::engraving::ElementType elementType)
{
    switch (elementType) {
    case ElementType::NOTEHEAD:
    case ElementType::NOTEDOT:
    case ElementType::FINGERING:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::BEND:
    case ElementType::TIE:
    case ElementType::PARTIAL_TIE:
    case ElementType::LAISSEZ_VIB:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXTLINE:
    case ElementType::NOTELINE:
    case ElementType::GLISSANDO:
    case ElementType::HAMMER_ON_PULL_OFF:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   Note::add
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::add(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped ? wrapped->element() : nullptr;
    if (s) {
        // Ensure that the object has the expected ownership
        if (wrapped->ownership() == Ownership::SCORE) {
            LOGW("Note::add: Cannot add this element. The element is already part of the score.");
            return;              // Don't allow operation.
        }
        // Score now owns the object.
        wrapped->setOwnership(Ownership::SCORE);

        addInternal(note(), s);
    }
}

//---------------------------------------------------------
//   Note::addInternal
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::addInternal(mu::engraving::Note* note, mu::engraving::EngravingItem* s)
{
    // Provide parentage for element.
    s->setScore(note->score());
    s->setParent(note);
    s->setTrack(note->track());

    if (s && isChildAllowed(s->type())) {
        // Create undo op and add the element.
        toScore(note->score())->undoAddElement(s);
    } else if (s) {
        LOGD("Note::add() not impl. %s", s->typeName());
    }
}

//---------------------------------------------------------
//   Note::remove
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::remove(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped->element();
    if (!s) {
        LOGW("PluginAPI::Note::remove: Unable to retrieve element. %s", qPrintable(wrapped->name()));
    } else if (s->explicitParent() != note()) {
        LOGW("PluginAPI::Note::remove: The element is not a child of this note. Use removeElement() instead.");
    } else if (isChildAllowed(s->type())) {
        note()->score()->deleteItem(s);     // Create undo op and remove the element.
    } else {
        LOGD("Note::remove() not impl. %s", s->typeName());
    }
}

//---------------------------------------------------------
//   DurationElement::ticks
//---------------------------------------------------------

FractionWrapper* DurationElement::ticks() const
{
    return wrap(durationElement()->ticks());
}

//---------------------------------------------------------
//   DurationElement::changeCRlen
//---------------------------------------------------------

void DurationElement::changeCRlen(FractionWrapper* len)
{
    if (!durationElement()->isChordRest()) {
        LOGW("DurationElement::changeCRlen: can only change length for chords or rests");
        return;
    }
    const mu::engraving::Fraction f = len->fraction();
    if (!f.isValid() || f.isZero() || f.negative()) {
        LOGW("DurationElement::changeCRlen: invalid parameter values: %s", qPrintable(f.toString()));
        return;
    }
    durationElement()->score()->changeCRlen(toChordRest(durationElement()), f);
}

//---------------------------------------------------------
//   DurationElement::globalDuration
//---------------------------------------------------------

FractionWrapper* DurationElement::globalDuration() const
{
    return wrap(durationElement()->globalTicks());
}

//---------------------------------------------------------
//   DurationElement::actualDuration
//---------------------------------------------------------

FractionWrapper* DurationElement::actualDuration() const
{
    return wrap(durationElement()->actualTicks());
}

//---------------------------------------------------------
//   DurationElement::parentTuplet
//---------------------------------------------------------

Tuplet* DurationElement::parentTuplet()
{
    return wrap<Tuplet>(durationElement()->tuplet());
}

//---------------------------------------------------------
//   Chord::setPlayEventType
//---------------------------------------------------------

void Chord::setPlayEventType(mu::engraving::PlayEventType v)
{
    // Only create undo operation if the value has changed.
    if (v != chord()->playEventType()) {
        chord()->score()->setPlaylistDirty();
        chord()->score()->undo(new ChangeChordPlayEventType(chord(), v));
    }
}

//---------------------------------------------------------
//   Chord::add
//---------------------------------------------------------

void Chord::add(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped ? wrapped->element() : nullptr;
    if (s) {
        // Ensure that the object has the expected ownership
        if (wrapped->ownership() == Ownership::SCORE) {
            LOGW("Chord::add: Cannot add this element. The element is already part of the score.");
            return;              // Don't allow operation.
        }
        // Score now owns the object.
        wrapped->setOwnership(Ownership::SCORE);

        addInternal(chord(), s);
    }
}

//---------------------------------------------------------
//   Chord::addInternal
//---------------------------------------------------------

void Chord::addInternal(mu::engraving::Chord* chord, mu::engraving::EngravingItem* s)
{
    // Provide parentage for element.
    s->setScore(chord->score());
    s->setParent(chord);
    // If a note, ensure the element has proper Tpc values. (Will crash otherwise)
    if (s->isNote()) {
        s->setTrack(chord->track());
        toNote(s)->setTpcFromPitch();
    }
    // Create undo op and add the element.
    chord->score()->undoAddElement(s);
}

//---------------------------------------------------------
//   Chord::remove
//---------------------------------------------------------

void Chord::remove(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped->element();
    if (!s) {
        LOGW("PluginAPI::Chord::remove: Unable to retrieve element. %s", qPrintable(wrapped->name()));
    } else if (s->explicitParent() != chord()) {
        LOGW("PluginAPI::Chord::remove: The element is not a child of this chord. Use removeElement() instead.");
    } else if (chord()->notes().size() <= 1 && s->type() == ElementType::NOTE) {
        LOGW("PluginAPI::Chord::remove: Removal of final note is not allowed.");
    } else {
        chord()->score()->deleteItem(s);     // Create undo op and remove the element.
    }
}

EngravingItem* Measure::vspacerUp(int staffIdx)
{
    return wrap(measure()->vspacerUp(static_cast<staff_idx_t>(staffIdx)));
}

EngravingItem* Measure::vspacerDown(int staffIdx)
{
    return wrap(measure()->vspacerDown(static_cast<staff_idx_t>(staffIdx)));
}

EngravingItem* Measure::noText(int staffIdx)
{
    return wrap(measure()->noText(static_cast<staff_idx_t>(staffIdx)));
}

EngravingItem* Measure::mmRangeText(int staffIdx)
{
    return wrap(measure()->mmRangeText(static_cast<staff_idx_t>(staffIdx)));
}

bool Measure::corrupted(int staffIdx)
{
    return measure()->corrupted(static_cast<staff_idx_t>(staffIdx));
}

bool Measure::visible(int staffIdx)
{
    return measure()->visible(static_cast<staff_idx_t>(staffIdx));
}

bool Measure::stemless(int staffIdx)
{
    return measure()->stemless(static_cast<staff_idx_t>(staffIdx));
}

FractionWrapper* MeasureBase::tick() const
{
    return wrap(measureBase()->tick());
}

FractionWrapper* MeasureBase::ticks() const
{
    return wrap(measureBase()->ticks());
}

void MeasureBase::add(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped ? wrapped->element() : nullptr;
    if (s) {
        // Ensure that the object has the expected ownership
        if (wrapped->ownership() == Ownership::SCORE) {
            LOGW("MeasureBase::add: Cannot add this element. The element is already part of the score.");
            return;              // Don't allow operation.
        }
        // Score now owns the object.
        wrapped->setOwnership(Ownership::SCORE);

        addInternal(measureBase(), s);
    }
}

void MeasureBase::addInternal(mu::engraving::MeasureBase* measureBase, mu::engraving::EngravingItem* s)
{
    s->setScore(measureBase->score());
    s->setParent(measureBase);
    measureBase->score()->undoAddElement(s);
}

void MeasureBase::remove(apiv1::EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped->element();
    if (!s) {
        LOGW("PluginAPI::MeasureBase::remove: Unable to retrieve element. %s", qPrintable(wrapped->name()));
    } else if (s->explicitParent() != measureBase()) {
        LOGW("PluginAPI::MeasureBase::remove: The element is not a child of this measure base. Use removeElement() instead.");
    } else {
        measureBase()->score()->deleteItem(s);     // Create undo op and remove the element.
    }
}

QRectF System::bbox(int staffIdx)
{
    mu::engraving::SysStaff* ss = muse::value(system()->staves(), static_cast<staff_idx_t>(staffIdx));
    return ss ? scaleRect(ss->bbox(), system()->spatium()) : QRectF();
}

qreal System::yOffset(int staffIdx)
{
    mu::engraving::SysStaff* ss = muse::value(system()->staves(), static_cast<staff_idx_t>(staffIdx));
    return ss ? ss->yOffset() / system()->spatium() : 0.0;
}

bool System::show(int staffIdx)
{
    mu::engraving::SysStaff* ss = muse::value(system()->staves(), static_cast<staff_idx_t>(staffIdx));
    return ss ? ss->show() : false;
}

void System::setIsLocked(bool locked)
{
    if (locked == isLocked()) {
        return;
    }
    const mu::engraving::SystemLock* currentLock = system()->systemLock();
    if (currentLock && !locked) {
        system()->score()->undoRemoveSystemLock(currentLock);
    } else if (!currentLock && locked) {
        system()->score()->undoAddSystemLock(new mu::engraving::SystemLock(system()->first(), system()->last()));
    }
}

//---------------------------------------------------------
//   Page::pagenumber
//---------------------------------------------------------

int Page::pagenumber() const
{
    return static_cast<int>(page()->no());
}

//---------------------------------------------------------
//   Staff::part
//---------------------------------------------------------

Part* Staff::part()
{
    return wrap<Part>(staff()->part());
}

FractionWrapper* Staff::timeStretch(FractionWrapper* tick)
{
    return wrap(staff()->timeStretch(tick->fraction()));
}

EngravingItem* Staff::timeSig(FractionWrapper* tick)
{
    return wrap(staff()->timeSig(tick->fraction()));
}

int Staff::key(FractionWrapper* tick)
{
    return int(staff()->key(tick->fraction()));
}

IntervalWrapper* Staff::transpose(FractionWrapper* tick)
{
    return wrap(staff()->transpose(tick->fraction()));
}

QVariantMap Staff::swing(FractionWrapper* f)
{
    mu::engraving::SwingParameters swingParams = staff()->swing(f->fraction());
    QVariantMap pluginSwingParams;
    pluginSwingParams["swingUnit"] = swingParams.swingUnit;
    pluginSwingParams["swingRatio"] = swingParams.swingRatio;
    pluginSwingParams["isOn"] = swingParams.isOn();
    return pluginSwingParams;
}

QVariantMap Staff::capo(FractionWrapper* f)
{
    const mu::engraving::CapoParams& capoParams = staff()->capo(f->fraction());
    QVariantMap pluginCapoParams;
    pluginCapoParams["active"] = capoParams.active;
    pluginCapoParams["fretPosition"] = capoParams.fretPosition;
    QVariantList ignoredStrings;
    ignoredStrings.reserve(static_cast<int>(capoParams.ignoredStrings.size()));
    for (size_t string : capoParams.ignoredStrings) {
        ignoredStrings.append(static_cast<int>(string));
    }
    pluginCapoParams["ignoredStrings"] = ignoredStrings;
    return pluginCapoParams;
}

bool Staff::stemless(FractionWrapper* tick)
{
    return staff()->stemless(tick->fraction());
}

qreal Staff::staffHeight(FractionWrapper* tick)
{
    return staff()->staffHeight(tick->fraction());
}

bool Staff::isPitchedStaff(FractionWrapper* tick)
{
    return staff()->isPitchedStaff(tick->fraction());
}

bool Staff::isTabStaff(FractionWrapper* tick)
{
    return staff()->isTabStaff(tick->fraction());
}

bool Staff::isDrumStaff(FractionWrapper* tick)
{
    return staff()->isDrumStaff(tick->fraction());
}

int Staff::lines(FractionWrapper* tick)
{
    return staff()->lines(tick->fraction());
}

qreal Staff::lineDistance(FractionWrapper* tick)
{
    return staff()->lineDistance(tick->fraction());
}

bool Staff::isLinesInvisible(FractionWrapper* tick)
{
    return staff()->isLinesInvisible(tick->fraction());
}

int Staff::middleLine(FractionWrapper* tick)
{
    return staff()->middleLine(tick->fraction());
}

int Staff::bottomLine(FractionWrapper* tick)
{
    return staff()->bottomLine(tick->fraction());
}

qreal Staff::staffMag(FractionWrapper* tick)
{
    return staff()->staffMag(tick->fraction());
}

qreal Staff::spatium(FractionWrapper* tick)
{
    return staff()->spatium(tick->fraction());
}

int Staff::pitchOffset(FractionWrapper* tick)
{
    return staff()->pitchOffset(tick->fraction());
}

bool Staff::isVoiceVisible(int voice)
{
    return staff()->isVoiceVisible(voice);
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps mu::engraving::EngravingItem choosing the correct wrapper type
///   at runtime based on the actual element type.
//---------------------------------------------------------

EngravingItem* mu::engraving::apiv1::wrap(mu::engraving::EngravingItem* e, Ownership own)
{
    if (!e) {
        return nullptr;
    }

    using mu::engraving::ElementType;
    switch (e->type()) {
    case ElementType::TIE:
    case ElementType::PARTIAL_TIE:
    case ElementType::LAISSEZ_VIB:
        return wrap<Tie>(toTie(e), own);
    case ElementType::NOTE:
        return wrap<Note>(toNote(e), own);
    case ElementType::CHORD:
        return wrap<Chord>(toChord(e), own);
    case ElementType::TUPLET:
        return wrap<Tuplet>(toTuplet(e), own);
    case ElementType::SEGMENT:
        return wrap<Segment>(toSegment(e), own);
    case ElementType::MEASURE:
        return wrap<Measure>(toMeasure(e), own);
    case ElementType::HBOX:
    case ElementType::VBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
        return wrap<MeasureBase>(toMeasureBase(e), own);
    case ElementType::SYSTEM:
        return wrap<System>(toSystem(e), own);
    case ElementType::PAGE:
        return wrap<Page>(toPage(e), own);
    default:
        if (e->isDurationElement()) {
            if (e->isChordRest()) {
                return wrap<ChordRest>(toChordRest(e), own);
            }
            return wrap<DurationElement>(toDurationElement(e), own);
        }
        if (e->isSpannerSegment()) {
            return wrap<SpannerSegment>(toSpannerSegment(e), own);
        }
        if (e->isSpanner()) {
            return wrap<Spanner>(toSpanner(e), own);
        }
        break;
    }
    return wrap<EngravingItem>(e, own);
}
