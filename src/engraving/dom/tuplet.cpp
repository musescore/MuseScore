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

#include "tuplet.h"

#include "style/textstyle.h"

#include "beam.h"
#include "chord.h"
#include "engravingitem.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   tupletStyle
//---------------------------------------------------------

static const ElementStyle tupletStyle {
    { Sid::tupletDirection,                    Pid::DIRECTION },
    { Sid::tupletNumberType,                   Pid::NUMBER_TYPE },
    { Sid::tupletBracketType,                  Pid::BRACKET_TYPE },
    { Sid::tupletBracketWidth,                 Pid::LINE_WIDTH },
    { Sid::tupletFontFace,                     Pid::FONT_FACE },
    { Sid::tupletFontSize,                     Pid::FONT_SIZE },
    { Sid::tupletFontStyle,                    Pid::FONT_STYLE },
    { Sid::tupletAlign,                        Pid::ALIGN },
    { Sid::tupletMinDistance,                  Pid::MIN_DISTANCE },
    { Sid::tupletFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
};

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Measure* parent)
    : DurationElement(ElementType::TUPLET, parent)
{
    m_direction    = DirectionV::AUTO;
    m_numberType   = TupletNumberType::SHOW_NUMBER;
    m_bracketType  = TupletBracketType::AUTO_BRACKET;
    m_ratio        = Fraction(1, 1);
    m_number       = 0;
    m_hasBracket   = false;
    m_isUp         = true;
    m_id           = 0;
    initElementStyle(&tupletStyle);
}

Tuplet::Tuplet(const Tuplet& t)
    : DurationElement(t)
{
    m_tick         = t.m_tick;
    m_hasBracket   = t.m_hasBracket;
    m_ratio        = t.m_ratio;
    m_baseLen      = t.m_baseLen;
    m_direction    = t.m_direction;
    m_numberType   = t.m_numberType;
    m_bracketType  = t.m_bracketType;
    m_bracketWidth = t.m_bracketWidth;

    m_isUp          = t.m_isUp;

    m_p1             = t.m_p1;
    m_p2             = t.m_p2;
    m_userP1            = t.m_userP1;
    m_userP2            = t.m_userP2;

    m_id            = t.m_id;
    // recreated on layout
    m_number = 0;
}

//---------------------------------------------------------
//   ~Tuplet
//---------------------------------------------------------

Tuplet::~Tuplet()
{
    m_beingDestructed = true;

    for (DurationElement* de : m_allElements) {
        assert(de->tuplet() == this);
        de->setTuplet(nullptr);
    }
    delete m_number;
}

void Tuplet::setParent(Measure* parent)
{
    EngravingItem::setParent(parent);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Tuplet::setSelected(bool f)
{
    EngravingItem::setSelected(f);
    if (m_number) {
        m_number->setSelected(f);
    }
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Tuplet::setVisible(bool f)
{
    EngravingItem::setVisible(f);
    if (m_number) {
        m_number->setVisible(f);
    }
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void Tuplet::setColor(const Color& col)
{
    EngravingItem::setColor(col);
    if (m_number) {
        m_number->setColor(col);
    }
}

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction Tuplet::rtick() const
{
    return tick() - measure()->tick();
}

bool Tuplet::isInRange(const Fraction& startTick, const Fraction& endTick) const
{
    const bool startsInRange = tick() >= startTick;
    const bool endsInRange = tick() + actualTicks() <= endTick;
    return startsInRange && endsInRange;
}

//---------------------------------------------------------
//   resetNumberProperty
//   reset number properties to default values
//   Set FONT_ITALIC to true, because for tuplets number should be italic
//---------------------------------------------------------

void Tuplet::resetNumberProperty()
{
    resetNumberProperty(m_number);
}

void Tuplet::resetNumberProperty(Text* number)
{
    for (auto p : { Pid::FONT_FACE, Pid::FONT_STYLE, Pid::FONT_SIZE, Pid::ALIGN, Pid::SIZE_SPATIUM_DEPENDENT }) {
        number->resetProperty(p);
    }
}

//---------------------------------------------------------
//   calcHasBracket
//---------------------------------------------------------

bool Tuplet::calcHasBracket(const DurationElement* cr1, const DurationElement* cr2) const
{
    if (m_bracketType != TupletBracketType::AUTO_BRACKET) {
        return m_bracketType != TupletBracketType::SHOW_NO_BRACKET;
    }
    if (cr1 == cr2) { // Degenerate tuplet
        return false;
    }
    if (!cr1->isChord() || !cr2->isChord()) {
        return true;
    }
    const Chord* c1 = toChord(cr1);
    const Chord* c2 = toChord(cr2);

    Beam const* beamStart = c1->beam();
    Beam const* beamEnd = c2->beam();
    if (!beamStart || !beamEnd || beamStart != beamEnd) {
        return true;
    }
    bool tupletStartsBeam = beamStart->elements().front() == c1;
    bool tupletEndsBeam = beamEnd->elements().back() == c2;
    bool headSide = isUp() != (c1->up() || c2->up());
    bool isCross = c1->vStaffIdx() != c2->vStaffIdx();
    if (tupletStartsBeam && tupletEndsBeam && (!headSide || isCross)) {
        return false;
    }

    int beamCount = -1;
    for (DurationElement* e : m_currentElements) {
        if (e->isTuplet() || e->isRest()) {
            return true;
        } else if (e->isChordRest()) {
            ChordRest* cr = toChordRest(e);
            if (cr->beam() == 0) {
                return true;
            }
            if (beamCount == -1) {
                beamCount = cr->beams();
            } else if (beamCount != cr->beams()) {
                return true;
            }
        }
    }
    if (beamCount < 1) {
        return true;
    }

    bool startChordBreaks32 = false;
    bool startChordBreaks64 = false;
    Chord* prevStartChord = c1->prev();
    if (prevStartChord) {
        beamStart->calcBeamBreaks(c1, prevStartChord, beamCount - 1, startChordBreaks32, startChordBreaks64);
    }
    bool startChordDefinesTuplet = startChordBreaks32 || startChordBreaks64 || tupletStartsBeam;
    if (prevStartChord) {
        startChordDefinesTuplet = startChordDefinesTuplet || prevStartChord->beams() < beamCount;
    }

    bool endChordBreaks32 = false;
    bool endChordBreaks64 = false;
    Chord* nextEndChord = c2->next();
    if (nextEndChord) {
        beamEnd->calcBeamBreaks(nextEndChord, c2, beamCount - 1, endChordBreaks32, endChordBreaks64);
    }
    bool endChordDefinesTuplet = endChordBreaks32 || endChordBreaks64 || tupletEndsBeam;
    if (nextEndChord) {
        endChordDefinesTuplet = endChordDefinesTuplet || nextEndChord->beams() < beamCount;
    }

    if (startChordDefinesTuplet && endChordDefinesTuplet && (!headSide || isCross)) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tuplet::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : scanChildren()) {
        if (child == m_number && !all) {
            continue; // don't scan number unless all is true
        }
        child->scanElements(data, func, all);
    }
    if (all || visible() || score()->isShowInvisible()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(EngravingItem* e)
{
#ifndef NDEBUG
    for (DurationElement* el : m_currentElements) {
        if (el == e) {
            LOGD("%p: %p %s already there", this, e, e->typeName());
            return;
        }
    }
#endif

    switch (e->type()) {
    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::TUPLET: {
        DurationElement* de = toDurationElement(e);
        de->setTuplet(this);

        bool found = false;
        Fraction tick = de->rtick();
        if (tick != Fraction(-1, 1)) {
            for (unsigned int i = 0; i < m_currentElements.size(); ++i) {
                if (m_currentElements[i]->rtick() > tick) {
                    m_currentElements.insert(m_currentElements.begin() + i, de);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            m_currentElements.push_back(de);
        }
    }
    break;

    default:
        LOGD("Tuplet::add() unknown element");
        return;
    }

    e->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Tuplet::remove(EngravingItem* e)
{
    switch (e->type()) {
//            case ElementType::TEXT:
//                  if (e == _number)
//                        _number = 0;
//                  break;
    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::TUPLET: {
        auto i = std::find(m_currentElements.begin(), m_currentElements.end(), toDurationElement(e));
        if (i == m_currentElements.end()) {
            LOGD("Tuplet::remove: cannot find element <%s>", e->typeName());
            LOGD("  elements %zu", m_currentElements.size());
        } else {
            m_currentElements.erase(i);
            e->removed();
        }
    }
    break;
    default:
        LOGD("Tuplet::remove: unknown element");
        break;
    }
}

void Tuplet::addDurationElement(DurationElement* de)
{
    assert(!m_beingDestructed);
    m_allElements.insert(de);
}

void Tuplet::removeDurationElement(DurationElement* de)
{
    if (m_beingDestructed) {
        return;
    }

    m_allElements.erase(de);
}

//---------------------------------------------------------
//   isEditable
//---------------------------------------------------------

bool Tuplet::isEditable() const
{
    return m_hasBracket;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Tuplet::startEditDrag(EditData& ed)
{
    DurationElement::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);

    eed->pushProperty(Pid::P1);
    eed->pushProperty(Pid::P2);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tuplet::editDrag(EditData& ed)
{
    if (ed.curGrip == Grip::START) {
        m_userP1 += ed.delta;
    } else {
        m_userP2 += ed.delta;
    }
    setGenerated(false);
    //layout();
    //score()->setUpdateAll();
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Tuplet::gripsPositions(const EditData&) const
{
    const PointF pp(pagePos());
    return { pp + m_p1, pp + m_p2 };
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tuplet::reset()
{
    undoChangeProperty(Pid::P1, PointF());
    undoChangeProperty(Pid::P2, PointF());
    EngravingItem::reset();
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Tuplet::dump() const
{
    EngravingItem::dump();
    LOGD() << "ratio: " << m_ratio.toString();
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Tuplet::setTrack(track_idx_t val)
{
    if (tuplet()) {
        tuplet()->setTrack(val);
    }
    if (m_number) {
        m_number->setTrack(val);
    }
    EngravingItem::setTrack(val);
}

//---------------------------------------------------------
//   tickGreater
//---------------------------------------------------------

static bool tickGreater(const DurationElement* a, const DurationElement* b)
{
    return a->tick() < b->tick();
}

//---------------------------------------------------------
//   sortElements
//---------------------------------------------------------

void Tuplet::sortElements()
{
    std::sort(m_currentElements.begin(), m_currentElements.end(), tickGreater);
}

//---------------------------------------------------------
//   cross
//---------------------------------------------------------

bool Tuplet::cross() const
{
    for (DurationElement* de : m_currentElements) {
        if (!de) {
            continue;
        } else if (de->isChordRest()) {
            if (toChordRest(de)->staffMove()) {
                return true;
            }
        } else if (de->isTuplet()) {
            if (toTuplet(de)->cross()) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   vStaffIdx
///  Staff index for layout based on the first ChordRest
//---------------------------------------------------------

staff_idx_t Tuplet::vStaffIdx() const
{
    if (elements().empty()) {
        return muse::nidx;
    }

    const DurationElement* cr = elements().front();
    if (!cr) {
        return muse::nidx;
    }

    while (cr->isTuplet()) {
        const Tuplet* t = toTuplet(cr);
        if (t->elements().empty()) {
            return muse::nidx;
        }
        cr = t->elements().front();
    }

    return cr->vStaffIdx();
}

//---------------------------------------------------------
//   elementsDuration
///  Get the sum of the element fraction in the tuplet,
///  even if the tuplet is not complete yet
//---------------------------------------------------------

Fraction Tuplet::elementsDuration()
{
    Fraction f;
    for (DurationElement* el : m_currentElements) {
        f += el->ticks();
    }
    return f;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Tuplet::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DIRECTION:
        return PropertyValue::fromValue<DirectionV>(m_direction);
    case Pid::NUMBER_TYPE:
        return int(m_numberType);
    case Pid::BRACKET_TYPE:
        return int(m_bracketType);
    case Pid::LINE_WIDTH:
        return m_bracketWidth;
    case Pid::NORMAL_NOTES:
        return m_ratio.denominator();
    case Pid::ACTUAL_NOTES:
        return m_ratio.numerator();
    case Pid::P1:
        return m_userP1;
    case Pid::P2:
        return m_userP2;
    case Pid::FONT_SIZE:
    case Pid::FONT_FACE:
    case Pid::FONT_STYLE:
    case Pid::ALIGN:
    case Pid::SIZE_SPATIUM_DEPENDENT:
        return m_number ? m_number->getProperty(propertyId) : PropertyValue();
    default:
        break;
    }
    return DurationElement::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tuplet::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::NUMBER_TYPE:
        setNumberType(TupletNumberType(v.toInt()));
        break;
    case Pid::BRACKET_TYPE:
        setBracketType(TupletBracketType(v.toInt()));
        break;
    case Pid::LINE_WIDTH:
        setBracketWidth(v.value<Spatium>());
        break;
    case Pid::NORMAL_NOTES:
        m_ratio.setDenominator(v.toInt());
        break;
    case Pid::ACTUAL_NOTES:
        m_ratio.setNumerator(v.toInt());
        break;
    case Pid::P1:
        m_userP1 = v.value<PointF>();
        break;
    case Pid::P2:
        m_userP2 = v.value<PointF>();
        break;
    case Pid::FONT_SIZE:
    case Pid::FONT_FACE:
    case Pid::FONT_STYLE:
    case Pid::ALIGN:
    case Pid::SIZE_SPATIUM_DEPENDENT:
        if (m_number) {
            m_number->setProperty(propertyId, v);
        }
        break;
    default:
        return DurationElement::setProperty(propertyId, v);
    }
    if (!m_currentElements.empty()) {
        m_currentElements.front()->triggerLayout();
        m_currentElements.back()->triggerLayout();
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Tuplet::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::TUPLET;
    case Pid::SYSTEM_FLAG:
        return false;
    case Pid::TEXT:
        return String();
    case Pid::NORMAL_NOTES:
    case Pid::ACTUAL_NOTES:
        return 0;
    case Pid::P1:
    case Pid::P2:
        return PointF();
    case Pid::ALIGN:
        return style().styleV(Sid::tupletAlign);
    case Pid::FONT_FACE:
        return style().styleV(Sid::tupletFontFace);
    case Pid::FONT_SIZE:
        return style().styleV(Sid::tupletFontSize);
    case Pid::FONT_STYLE:
        return style().styleV(Sid::tupletFontStyle);
    case Pid::SIZE_SPATIUM_DEPENDENT:
        return style().styleV(Sid::tupletFontSpatiumDependent);
    default:
        break;
    }

    // TODO: what does this do? Why TextStyleType::DEFAULT?
    for (const auto& spp : *textStyle(TextStyleType::DEFAULT)) {
        if (spp.pid == id) {
            return styleValue(id, spp.sid);
        }
    }

    return DurationElement::propertyDefault(id);
}

//---------------------------------------------------------
//   sanitizeTuplet
///    Check validity of tuplets and coherence between duration
///    and baselength. Needed for importing old files due to a bug
///    in the released version for corner-case tuplets.
///    See issue #136406 and Pull request #2881
//---------------------------------------------------------

void Tuplet::sanitizeTuplet()
{
    if (ratio().numerator() == ratio().reduced().numerator()) { // return if the ratio is an irreducible fraction
        return;
    }
    Fraction baseLenDuration = (Fraction(ratio().denominator(), 1) * baseLen().fraction()).reduced();

    // Due to a bug present in 2.1 (and before), a tuplet with non-reduced ratio could be
    // in a corrupted state (mismatch between duration and base length).
    // A tentative will now be made to retrieve the correct duration by summing up all the
    // durations of the elements constituting the tuplet. This does not work for
    // not-completely filled tuplets, such as tuplets in voices > 0 with
    // gaps (for example, a tuplet in second voice with a deleted chordrest element)

    Fraction testDuration(0, 1);
    for (DurationElement* de : elements()) {
        if (!de) {
            continue;
        }
        Fraction elementDuration(0, 1);
        if (de->isTuplet()) {
            Tuplet* t = toTuplet(de);
            t->sanitizeTuplet();
            elementDuration = t->ticks();
        } else {
            elementDuration = de->ticks();
        }
        testDuration += elementDuration;
    }
    testDuration = testDuration / ratio();
    testDuration.reduce();
    if (elements().back()->tick() + elements().back()->actualTicks() - elements().front()->tick() > testDuration) {
        return;         // this tuplet has missing elements; do not sanitize
    }
    if (!(testDuration == baseLenDuration && baseLenDuration == ticks())) {
        Fraction f = testDuration * Fraction(1, ratio().denominator());
        f.reduce();
        Fraction fbl(1, f.denominator());
        if (TDuration::isValid(fbl)) {
            setTicks(testDuration);
            setBaseLen(fbl);
            LOGD("Tuplet %p sanitized duration %d/%d   baseLen %d/%d", this,
                 testDuration.numerator(), testDuration.denominator(),
                 1, fbl.denominator());
        } else {
            LOGD("Impossible to sanitize the tuplet");
        }
    }
}

//---------------------------------------------------------
//   addMissingElement
//     Add a rest with the given start and end ticks.
//     Should only be called from Tuplet::addMissingElements().
//     Needed for importing files that saved incomplete tuplets.
//---------------------------------------------------------

Fraction Tuplet::addMissingElement(const Fraction& startTick, const Fraction& endTick)
{
    Fraction f = (endTick - startTick) * ratio();
    TDuration d = TDuration(f, true);
    if (!d.isValid()) {
        LOGD("Tuplet::addMissingElement(): invalid duration: %d/%d", f.numerator(), f.denominator());
        return Fraction::fromTicks(0);
    }
    f = d.fraction();
    Segment* segment = measure()->getSegment(SegmentType::ChordRest, startTick);
    Rest* rest = Factory::createRest(segment);
    rest->setDurationType(d);
    rest->setTicks(f);
    rest->setTrack(track());
    rest->setVisible(false);

    segment->add(rest);
    add(rest);
    return f;
}

//---------------------------------------------------------
//   addMissingElements
//     Make this tuplet complete by filling in holes where
//     there ought to be rests. Needed for importing files
//     that saved incomplete tuplets.
//---------------------------------------------------------

void Tuplet::addMissingElements()
{
    if (tuplet()) {
        return;         // do not correct nested tuplets
    }
    if (voice() == 0) {
        return;         // nothing to do for tuplets in voice 1
    }

    Fraction missingElementsDuration = ticks() * ratio() - elementsDuration();
    if (missingElementsDuration.isZero()) {
        return;
    }

    // first, fill in any holes in the middle of the tuplet
    Fraction expectedTick = elements().front()->tick();

    const std::vector<DurationElement*> elementsCopy = elements(); // mofified during loop
    for (const DurationElement* de : elementsCopy) {
        if (!de) {
            continue;
        }
        if (de->tick() != expectedTick) {
            missingElementsDuration -= addMissingElement(expectedTick, de->tick());
            if (missingElementsDuration.isZero()) {
                return;
            }
        }
        expectedTick += de->actualTicks();
    }

    // calculate the tick where we would expect a tuplet of this duration to start
    // TODO: check:
    expectedTick = elements().front()->tick() - Fraction::fromTicks(elements().front()->tick().ticks() % ticks().ticks());
    if (expectedTick != elements().front()->tick()) {
        // try to fill a hole at the beginning of the tuplet
        Fraction firstAvailableTick = measure()->tick();
        Segment* segment = measure()->findSegment(SegmentType::ChordRest, elements().front()->tick());
        ChordRest* prevChordRest = segment && segment->prev() ? segment->prev()->nextChordRest(track(), true) : nullptr;
        if (prevChordRest && prevChordRest->measure() == measure()) {
            firstAvailableTick = prevChordRest->tick() + prevChordRest->actualTicks();
        }
        if (firstAvailableTick != elements().front()->tick()) {
            Fraction f = missingElementsDuration / ratio();
            Fraction ticksRequired = f;
            Fraction endTick = elements().front()->tick();
            Fraction startTick = std::max(firstAvailableTick, endTick - ticksRequired);
            if (expectedTick > startTick) {
                startTick = expectedTick;
            }
            missingElementsDuration -= addMissingElement(startTick, endTick);
            if (missingElementsDuration.isZero()) {
                return;
            }
        }
    }
    // now fill a hole at the end of the tuplet
    Fraction startTick = elements().back()->tick() + elements().back()->actualTicks();
    Fraction endTick = elements().front()->tick() + ticks();
    // just to be safe, find the next ChordRest in the track, and adjust endTick if necessary
    Segment* segment = measure()->findSegment(SegmentType::ChordRest, elements().back()->tick());
    ChordRest* nextChordRest = segment && segment->next() ? segment->next()->nextChordRest(track(), false) : nullptr;
    if (nextChordRest && nextChordRest->tick() < endTick) {
        endTick = nextChordRest->tick();
    }
    missingElementsDuration -= addMissingElement(startTick, endTick);
    if (!missingElementsDuration.isZero()) {
        LOGD("Tuplet::addMissingElements(): still missing duration of %d/%d",
             missingElementsDuration.numerator(), missingElementsDuration.denominator());
    }
}

int Tuplet::computeTupletDenominator(int numerator, Fraction totalDuration)
{
    if (numerator <= 0) {
        return 0;
    }
    Fraction noteDuration = totalDuration / numerator;
    Fraction baseLen(1, 1);
    Fraction ratio = (totalDuration / baseLen).reduced();
    while (ratio.denominator() != 1 || baseLen / 2 >= noteDuration) {
        baseLen /= 2;
        ratio = (totalDuration / baseLen).reduced();
    }
    return ratio.numerator();
}

EngravingItem* Tuplet::nextElement()
{
    ChordRest* firstElement = toChordRest(elements().front());
    if (firstElement->type() == ElementType::CHORD) {
        Chord* chord = toChord(firstElement);
        return chord->firstGraceOrNote();
    }
    return firstElement;
}

EngravingItem* Tuplet::prevElement()
{
    ChordRest* firstElement = toChordRest(elements().front());
    staff_idx_t staffId = firstElement->staffIdx();
    EngravingItem* prevItem = firstElement->segment()->prevElement(staffId);
    return prevItem;
}
} // namespace mu::engraving
