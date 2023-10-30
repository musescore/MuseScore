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

#include "accidental.h"
#include "chord.h"
#include "editdata.h"
#include "guitarbend.h"
#include "note.h"
#include "part.h"
#include "score.h"
#include "staff.h"
#include "tie.h"
#include "utils.h"

namespace mu::engraving {
/****************************************
 *              GuitarBend
 * **************************************/

GuitarBend::GuitarBend(EngravingItem* parent)
    : SLine(ElementType::GUITAR_BEND, parent, ElementFlag::MOVABLE)
{
}

GuitarBend::GuitarBend(const GuitarBend& g)
    : SLine(g)
{
    _type = g.type();
}

GuitarBend::~GuitarBend()
{
    if (m_holdLine) {
        delete m_holdLine;
    }
}

LineSegment* GuitarBend::createLineSegment(System* parent)
{
    GuitarBendSegment* seg = new GuitarBendSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    return seg;
}

Note* GuitarBend::startNote() const
{
    EngravingItem* startEl = startElement();
    IF_ASSERT_FAILED(startEl && startEl->isNote()) {
        return nullptr;
    }
    return toNote(startEl);
}

void GuitarBend::setStartNotePitch(int pitch)
{
    Note* note = startNote();
    score()->undoChangePitch(note, pitch, note->tpc1(), note->tpc2());

    computeBendAmount();

    triggerLayout();
}

Note* GuitarBend::startNoteOfChain() const
{
    Note* startOfChain = startNote();
    GuitarBend* prevBend = findPrecedingBend();
    while (startOfChain && prevBend) {
        startOfChain = prevBend->startNote();
        prevBend = prevBend->findPrecedingBend();
    }

    return startOfChain;
}

Note* GuitarBend::endNote() const
{
    EngravingItem* endEl = endElement();
    IF_ASSERT_FAILED(endEl && endEl->isNote()) {
        return nullptr;
    }
    return toNote(endEl);
}

void GuitarBend::setEndNotePitch(int pitch)
{
    Note* note = endNote();

    Fraction tick = note->tick();
    Staff* staff = note->staff();

    Key key = staff->key(tick);
    Interval interval = staff->transpose(tick);
    interval.flip();

    int targetTpc1 = pitch2tpc(pitch, key, Prefer::NEAREST);
    int targetTpc2 = transposeTpc(targetTpc1, interval, true);

    score()->undoChangePitch(note, pitch, targetTpc1, targetTpc2);

    computeBendAmount();

    triggerLayout();
}

bool GuitarBend::isReleaseBend() const
{
    return endNote()->pitch() < startNote()->pitch();
}

bool GuitarBend::isFullRelease() const
{
    return isReleaseBend() && totBendAmountIncludingPrecedingBends() == 0;
}

void GuitarBend::fixNotesFrettingForStandardBend(Note* startNote, Note* endNote)
{
    Staff* curStaff =  startNote->staff();
    Part* curPart = startNote->part();
    if (!curStaff || !curPart) {
        return;
    }

    const StringData* stringData = curPart->stringData(startNote->tick());
    int startFret = startNote->fret();
    int startString = startNote->string();
    if (startFret == 0) {
        // Bend can't start from empty string
        startString += 1;
        startFret = stringData->fret(startNote->pitch(), startString, curStaff);
        startNote->undoChangeProperty(Pid::STRING, startString);
        startNote->undoChangeProperty(Pid::FRET, startFret);
    }

    if (endNote->string() != startString) {
        endNote->undoChangeProperty(Pid::STRING, startString);
        int endFret = stringData->fret(endNote->pitch(), startString, curStaff);
        endNote->undoChangeProperty(Pid::FRET, endFret);
    }
}

void GuitarBend::fixNotesFrettingForGraceBend(Note* grace, Note* main)
{
    // The start grace-note of bend must be on the same string as the main note
    int mainString = main->string();
    const StringData* stringData = main->part()->stringData(main->tick());
    int graceFret = stringData->fret(grace->pitch(), mainString, main->staff());
    if (graceFret > 0) {
        // There is valid fretting
        grace->undoChangeProperty(Pid::STRING, mainString);
        grace->undoChangeProperty(Pid::FRET, graceFret);
    } else {
        // No valid fretting on this string, so it must be played on the lower string
        mainString += 1;
        main->undoChangeProperty(Pid::STRING, mainString);
        int mainFret = stringData->fret(main->pitch(), mainString, main->staff());
        main->undoChangeProperty(Pid::FRET, mainFret);

        grace->undoChangeProperty(Pid::STRING, mainString);
        graceFret = stringData->fret(grace->pitch(), mainString, main->staff());
        grace->undoChangeProperty(Pid::FRET, graceFret);
    }
}

PropertyValue GuitarBend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::DIRECTION:
        return direction();
    case Pid::BEND_SHOW_HOLD_LINE:
        return showHoldLine();
    default:
        return SLine::getProperty(id);
    }
}

bool GuitarBend::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::BEND_SHOW_HOLD_LINE:
        setShowHoldLine(v.toBool());
        break;
    default:
        return SLine::setProperty(propertyId, v);
    }
    triggerLayoutAll();
    return true;
}

PropertyValue GuitarBend::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::DIRECTION:
        return DirectionV::AUTO;
    case Pid::BEND_SHOW_HOLD_LINE:
        return true;
    default:
        return SLine::propertyDefault(id);
    }
}

void GuitarBend::computeBendAmount()
{
    if (type() == GuitarBendType::SLIGHT_BEND) {
        setBendAmountInQuarterTones(1);
        computeBendText();
        return;
    }

    Note* startN = startNote();
    Note* endN = endNote();

    int startPitch = startN->pitch();
    int endPitch = endN->pitch();

    int pitchDiffInQuarterTones = 2 * (endPitch - startPitch);

    Accidental* startAcc = startN->accidental();
    Accidental* endAcc = endN->accidental();
    int startCentsOff = startAcc ? Accidental::subtype2centOffset(startAcc->accidentalType()) : 0;
    int endCentsOff = endAcc ? Accidental::subtype2centOffset(endAcc->accidentalType()) : 0;

    int startOffInQuarterTones = round(double(startCentsOff) / 50);
    int endOffInQuarterTones = round(double(endCentsOff) / 50);

    pitchDiffInQuarterTones += endOffInQuarterTones - startOffInQuarterTones;

    setBendAmountInQuarterTones(pitchDiffInQuarterTones);

    computeBendText();
}

int GuitarBend::totBendAmountIncludingPrecedingBends() const
{
    int bendAmount = bendAmountInQuarterTones();

    GuitarBend* prevBend = findPrecedingBend();
    while (prevBend) {
        bendAmount += prevBend->bendAmountInQuarterTones();
        prevBend = prevBend->findPrecedingBend();
    }

    return bendAmount;
}

void GuitarBend::computeBendText()
{
    int quarters = totBendAmountIncludingPrecedingBends();

    int fulls = quarters / 4;
    int quarts = quarters % 4;

    String string;
    if (fulls != 0) {
        string += String::number(fulls);
    }

    switch (quarts) {
    case 1:
        string += u"\u00BC";
        break;
    case 2:
        string += u"\u00BD";
        break;
    case 3:
        string += u"\u00BE";
        break;
    default:
        break;
    }

    if (string == u"1" && style().styleB(Sid::guitarBendUseFull)) {
        string = String(u"full");
    }

    mutldata()->setBendDigit(string);
}

GuitarBend* GuitarBend::findPrecedingBend() const
{
    Note* startN = startNote();
    while (startN->tieBack() && startN->tieBack()->startNote()) {
        startN = startN->tieBack()->startNote();
    }

    GuitarBend* precedingBend = startN->bendBack();
    if (precedingBend && precedingBend->type() != GuitarBendType::SLIGHT_BEND) {
        return precedingBend;
    }

    return nullptr;
}

void GuitarBend::updateHoldLine()
{
    bool needsHoldLine = false;

    Note* startOfHold = nullptr;
    Note* endOfHold = nullptr;

    if (!isFullRelease()) {
        startOfHold = endNote();
        endOfHold = startOfHold;

        while (endOfHold->tieFor() && endOfHold->tieFor()->endNote()) {
            endOfHold = endOfHold->tieFor()->endNote();
        }

        needsHoldLine = endOfHold != startOfHold;
    }

    if (!needsHoldLine || !showHoldLine()) {
        if (m_holdLine) {
            if (!m_holdLine->segmentsEmpty()) {
                m_holdLine->eraseSpannerSegments();
            }
            delete m_holdLine;
            m_holdLine = nullptr;
        }
        return;
    }

    if (!m_holdLine) {
        m_holdLine = new GuitarBendHold(this);
    }

    m_holdLine->setAnchor(Spanner::Anchor::NOTE);
    m_holdLine->setStartElement(startOfHold);
    m_holdLine->setEndElement(endOfHold);
    m_holdLine->setTick(startOfHold->tick());
    m_holdLine->setTick2(endOfHold->tick());
    m_holdLine->setTrack(track());
    m_holdLine->setTrack2(track());
    m_holdLine->setGenerated(true);
    m_holdLine->setParent(this);
}

double GuitarBend::lineWidth() const
{
    return style().styleMM(Sid::guitarBendLineWidth);
}

/****************************************
 *              GuitarBendSegment
 * **************************************/

GuitarBendSegment::GuitarBendSegment(GuitarBend* sp, System* parent)
    : LineSegment(ElementType::GUITAR_BEND_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
    m_text = new GuitarBendText(this);
    setFlag(ElementFlag::ON_STAFF, true);
}

GuitarBendSegment::GuitarBendSegment(const GuitarBendSegment& s)
    : LineSegment(s)
{
    m_vertexPointOff = s.m_vertexPointOff;
    m_text = new GuitarBendText(this);
}

GuitarBendSegment::~GuitarBendSegment()
{
    delete m_text;
}

std::vector<PointF> GuitarBendSegment::gripsPositions(const EditData&) const
{
    std::vector<PointF> grips(gripsCount());
    PointF pp(pagePos());
    grips[int(Grip::START)] = pp;
    grips[int(Grip::END)] = pos2() + pp;
    grips[int(Grip::MIDDLE)] = pos2() * .5 + pp;
    grips[int(Grip::APERTURE)] = ldata()->vertexPoint() + vertexPointOff() + pp;
    return grips;
}

void GuitarBendSegment::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    eed->pushProperty(Pid::BEND_VERTEX_OFF);
    LineSegment::startEditDrag(ed);
}

void GuitarBendSegment::editDrag(EditData& ed)
{
    PointF delta = ed.evtDelta;

    switch (ed.curGrip) {
    case Grip::START:
        setOffset(offset() + delta);
        setUserOff2(userOff2() - delta);
        setVertexPointOff(vertexPointOff() - delta);
        break;
    case Grip::END:
        setUserOff2(userOff2() + delta);
        break;
    case Grip::APERTURE:
        setVertexPointOff(vertexPointOff() + delta);
        break;
    case Grip::MIDDLE:
        setOffset(offset() + delta);
        setOffsetChanged(true);
        break;
    default:
        break;
    }

    triggerLayout();
}

PropertyValue GuitarBendSegment::getProperty(Pid id) const
{
    switch (id) {
    case Pid::BEND_VERTEX_OFF:
        return vertexPointOff();
    default:
        return LineSegment::getProperty(id);
    }
}

bool GuitarBendSegment::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::BEND_VERTEX_OFF:
        setVertexPointOff(v.value<PointF>());
        break;
    default:
        return LineSegment::setProperty(propertyId, v);
    }
    triggerLayoutAll();
    return true;
}

PropertyValue GuitarBendSegment::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BEND_VERTEX_OFF:
        return PointF();
    default:
        return LineSegment::propertyDefault(id);
    }
}

EngravingItem* GuitarBendSegment::propertyDelegate(Pid id)
{
    switch (id) {
    case Pid::DIRECTION:
        return guitarBend();
    case Pid::BEND_SHOW_HOLD_LINE:
        return guitarBend();
    default:
        return nullptr;
    }
}

void GuitarBendSegment::reset()
{
    undoResetProperty(Pid::DIRECTION);
    undoResetProperty(Pid::BEND_VERTEX_OFF);
    LineSegment::reset();
}

double GuitarBendSegment::lineWidth() const
{
    return guitarBend()->lineWidth();
}

void GuitarBendSegment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    func(data, m_text);
    LineSegment::scanElements(data, func, all);
}

Shape GuitarBendSegment::doCreateShape() const
{
    Shape shape;
    shape.add(ldata()->bbox(), this);
    if (!bendText()->empty()) {
        shape.add(bendText()->shape().translated(bendText()->pos()));
    }

    return shape;
}

bool GuitarBendSegment::isUserModified() const
{
    bool modified = !vertexPointOff().isNull() || (m_text && m_text->isUserModified());
    return modified || LineSegment::isUserModified();
}

/****************************************
 *            GuitarBendHold
 * **************************************/

GuitarBendHold::GuitarBendHold(GuitarBend* parent)
    : SLine(ElementType::GUITAR_BEND_HOLD, parent, ElementFlag::MOVABLE)
{
}

GuitarBendHold::GuitarBendHold(const GuitarBendHold& h)
    : SLine(h)
{
}

LineSegment* GuitarBendHold::createLineSegment(System* parent)
{
    GuitarBendHoldSegment* seg = new GuitarBendHoldSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    return seg;
}

Note* GuitarBendHold::startNote() const
{
    EngravingItem* startEl = startElement();
    assert(startEl && startEl->isNote());
    return toNote(startEl);
}

Note* GuitarBendHold::endNote() const
{
    EngravingItem* endEl = endElement();
    assert(endEl && endEl->isNote());
    return toNote(endEl);
}

double GuitarBendHold::lineWidth() const
{
    return style().styleMM(Sid::guitarBendLineWidth);
}

/****************************************
 *         GuitarBendHoldSegment
 * **************************************/

GuitarBendHoldSegment::GuitarBendHoldSegment(GuitarBendHold* sp, System* parent)
    : LineSegment(ElementType::GUITAR_BEND_HOLD_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
    setFlag(ElementFlag::ON_STAFF, true);
}

void GuitarBendHoldSegment::editDrag(EditData& ed)
{
    PointF delta = ed.evtDelta;

    switch (ed.curGrip) {
    case Grip::START:
        setOffset(offset() + delta);
        setUserOff2(userOff2() - delta);
        break;
    case Grip::END:
        setUserOff2(userOff2() + delta);
        break;
    case Grip::MIDDLE:
        setOffset(offset() + delta);
        setOffsetChanged(true);
        break;
    default:
        break;
    }

    triggerLayout();
}

/****************************************
 *         GuitarBendText
 * **************************************/

static const ElementStyle guitarBendStyle {};

GuitarBendText::GuitarBendText(GuitarBendSegment* parent)
    : TextBase(ElementType::GUITAR_BEND_TEXT, parent, TextStyleType::BEND)
{
    initElementStyle(&guitarBendStyle);
}
} // namespace mu::engraving
