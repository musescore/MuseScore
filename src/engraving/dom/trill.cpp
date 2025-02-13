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

#include "trill.h"

#include <cmath>

#include "types/typesconv.h"

#include "iengravingfont.h"

#include "accidental.h"
#include "chord.h"
#include "factory.h"
#include "ornament.h"
#include "score.h"
#include "system.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   trillStyle
//---------------------------------------------------------

static const ElementStyle trillStyle {
    { Sid::trillPlacement, Pid::PLACEMENT },
    { Sid::trillPosAbove,  Pid::OFFSET },
};

TrillSegment::TrillSegment(Trill* sp, System* parent)
    : LineSegment(ElementType::TRILL_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

TrillSegment::TrillSegment(System* parent)
    : LineSegment(ElementType::TRILL_SEGMENT, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TrillSegment::remove(EngravingItem* e)
{
    if (trill()->accidental() == e) {
        // accidental is part of trill
        trill()->setAccidental(nullptr);
        e->removed();
    }
}

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void TrillSegment::symbolLine(SymId start, SymId fill)
{
    double x1 = 0.0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    m_symbols.clear();
    m_symbols.push_back(start);
    double w1 = f->advance(start, mag);
    double w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        m_symbols.push_back(fill);
    }
    RectF r(f->bbox(m_symbols, mag));
    setbbox(r);
}

void TrillSegment::symbolLine(SymId start, SymId fill, SymId end)
{
    double x1 = 0.0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    m_symbols.clear();
    m_symbols.push_back(start);
    double w1 = f->advance(start, mag);
    double w2 = f->advance(fill, mag);
    double w3 = f->advance(end, mag);
    int n    = lrint((w - w1 - w3) / w2);
    for (int i = 0; i < n; ++i) {
        m_symbols.push_back(fill);
    }
    m_symbols.push_back(end);
    RectF r(f->bbox(m_symbols, mag));
    setbbox(r);
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TrillSegment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool)
{
    func(data, this);
    if (isSingleType() || isBeginType()) {
        Accidental* a = trill()->accidental();
        if (a) {
            func(data, a);
        }
        Chord* cueNoteChord = trill()->cueNoteChord();
        if (cueNoteChord) {
            cueNoteChord->scanElements(data, func);
        }
    }
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* TrillSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::TRILL_TYPE || pid == Pid::ORNAMENT_STYLE || pid == Pid::PLACEMENT) {
        return spanner();
    }
    return LineSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TrillSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return spanner()->placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
    }
    return LineSegment::getPropertyStyle(pid);
}

Sid Trill::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
    }
    return SLine::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(EngravingItem* parent)
    : SLine(ElementType::TRILL, parent)
{
    m_trillType     = TrillType::TRILL_LINE;
    m_ornament = nullptr;
    m_accidental = nullptr;
    m_cueNoteChord = nullptr;
    m_ornamentStyle = OrnamentStyle::DEFAULT;
    initElementStyle(&trillStyle);
}

Trill::Trill(const Trill& t)
    : SLine(t)
{
    m_trillType = t.m_trillType;
    m_ornament = t.m_ornament ? t.m_ornament->clone() : nullptr;
    m_ornamentStyle = t.m_ornamentStyle;
    initElementStyle(&trillStyle);
}

EngravingItem* Trill::linkedClone()
{
    Trill* linkedTrill = clone();
    Ornament* linkedOrnament = toOrnament(m_ornament->linkedClone());
    linkedTrill->setOrnament(linkedOrnament);
    linkedTrill->setAutoplace(true);
    score()->undo(new Link(linkedTrill, this));
    return linkedTrill;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(EngravingItem* e)
{
    if (e == m_accidental) {
        m_accidental = nullptr;
        e->removed();
    }
}

void Trill::setTrack(track_idx_t n)
{
    EngravingItem::setTrack(n);

    for (SpannerSegment* ss : spannerSegments()) {
        ss->setTrack(n);
    }

    if (m_ornament) {
        m_ornament->setTrack(n);
    }
}

void Trill::setScore(Score* s)
{
    Spanner::setScore(s);
    if (m_ornament) {
        m_ornament->setScore(s);
    }
}

void Trill::computeStartElement()
{
    Spanner::computeStartElement();
    if (startElement() && startElement()->isChord() && m_ornament) {
        m_ornament->setParent(startElement());
    }
}

PointF Trill::trillLinePos(const SLine* line, Grip grip, System** system)
{
    if (!line) {
        return PointF();
    }

    bool start = grip == Grip::START;
    bool mmRest = line->style().styleB(Sid::createMultiMeasureRests);
    double graceOffset = 0.0;
    double clefOffset = 0.0;

    Segment* segment = start ? line->startSegment() : line->endSegment();
    if (!segment) {
        return PointF();
    }

    if (start) {
        *system = segment->measure()->system();
        double x = segment->x() + segment->measure()->x();
        return PointF(x, 0.0);
    }

    Segment* graceNoteSeg = segment->preAppendedItem(line->track2()) ? segment : nullptr;
    Segment* clefSeg = segment->isClefType() ? segment : nullptr;
    Fraction curTick = segment->tick();
    while (true) {
        Segment* prevSeg = mmRest ? segment->prev1MM() : segment->prev1();
        if (prevSeg && prevSeg->tick() == curTick) {
            graceNoteSeg = prevSeg->preAppendedItem(line->track2()) ? prevSeg : graceNoteSeg;
            clefSeg = prevSeg->isClefType() ? prevSeg : clefSeg;
            segment = prevSeg;
        } else {
            break;
        }
    }

    // Stop line before clefs
    if (clefSeg) {
        EngravingItem* clefItem = clefSeg->element(line->track2());
        if (clefItem && clefItem->isClef()) {
            Clef* clef = toClef(clefItem);
            SymId clefSym = ClefInfo::symId(clef->clefType());
            Shape clefShape = line->symShapeWithCutouts(clefSym).translated(clef->pos());
            clefOffset = segment->pageX() - clef->pageX() + clefShape.leftMostEdgeAtTop();
        }
    }

    // Stop line before grace notes
    if (graceNoteSeg) {
        const EngravingItem* preAppendedItem = graceNoteSeg->preAppendedItem(line->track2());
        if (preAppendedItem && preAppendedItem->isGraceNotesGroup()) {
            // get x position of leftmost grace note
            const Chord* leftMostGraceChord = nullptr;
            const GraceNotesGroup* graceGroup = toGraceNotesGroup(preAppendedItem);
            for (const Chord* graceChord : *graceGroup) {
                leftMostGraceChord = leftMostGraceChord
                                     && leftMostGraceChord->x() < graceChord->x() ? leftMostGraceChord : graceChord;
            }
            if (leftMostGraceChord) {
                graceOffset = segment->pageX() - leftMostGraceChord->pageX();
            }
        }
    }

    double offset = std::max(graceOffset, clefOffset);

    *system = segment->measure()->system();
    double x = segment->x() + segment->measure()->x() - line->spatium() - offset;
    return PointF(x, 0.0);
}

PointF Trill::linePos(Grip grip, System** system) const
{
    return trillLinePos(this, grip, system);
}

void Trill::setTrillType(TrillType tt)
{
    m_trillType = tt;
    if (!m_ornament) {
        // ornament parent will be explicitely set at layout stage
        m_ornament = Factory::createOrnament((ChordRest*)score()->dummy()->chord());
    }
    m_ornament->setTrack(track());
    m_ornament->setSymId(Ornament::fromTrillType(tt));
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle trillSegmentStyle {
    { Sid::trillPosAbove, Pid::OFFSET },
    { Sid::trillMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Trill::createLineSegment(System* parent)
{
    TrillSegment* seg = new TrillSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(lineColor());
    seg->initElementStyle(&trillSegmentStyle);
    return seg;
}

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

String Trill::trillTypeUserName() const
{
    return TConv::translatedUserName(trillType());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Trill::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        return int(trillType());
    case Pid::ORNAMENT_STYLE:
        return ornamentStyle();
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Trill::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        setTrillType(TrillType(val.toInt()));
        break;
    case Pid::ORNAMENT_STYLE:
        setOrnamentStyle(val.value<OrnamentStyle>());
        break;
    case Pid::COLOR:
        setColor(val.value<Color>());
        [[fallthrough]];
    default:
        if (!SLine::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Trill::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        return 0;
    case Pid::ORNAMENT_STYLE:
        return OrnamentStyle::DEFAULT;
    case Pid::PLACEMENT:
        return style().styleV(Sid::trillPlacement);

    default:
        return SLine::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Trill::subtypeUserName() const
{
    return TConv::userName(trillType());
}

muse::TranslatableString TrillSegment::subtypeUserName() const
{
    return trill()->subtypeUserName();
}

int TrillSegment::subtype() const
{
    return trill()->subtype();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Trill::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), trillTypeUserName());
}

void Trill::doComputeEndElement()
{
    setEndElement(score()->findChordRestEndingBeforeTickInStaffAndVoice(tick2(), track2staff(track2()), voice()));
}
}
