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

#include "barline.h"

#include "translation.h"
#include "draw/font.h"
#include "draw/fontmetrics.h"

#include "score.h"
#include "scorefont.h"
#include "sym.h"
#include "staff.h"
#include "part.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "articulation.h"
#include "stafftype.h"
#include "xml.h"
#include "marker.h"
#include "stafflines.h"
#include "spanner.h"
#include "undo.h"
#include "fermata.h"
#include "symbol.h"
#include "image.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   undoChangeBarLineType
//---------------------------------------------------------

static void undoChangeBarLineType(BarLine* bl, BarLineType barType, bool allStaves)
{
    Measure* m = bl->measure();
    if (!m) {
        return;
    }

    if (barType == BarLineType::START_REPEAT) {
        m = m->nextMeasure();
        if (!m) {
            return;
        }
    } else if (bl->barLineType() == BarLineType::START_REPEAT) {
        if (barType != BarLineType::END_REPEAT) {
            m->undoChangeProperty(Pid::REPEAT_START, false);
        }
        m = m->prevMeasure();
        if (!m) {
            return;
        }
    }

    switch (barType) {
    case BarLineType::END:
    case BarLineType::NORMAL:
    case BarLineType::DOUBLE:
    case BarLineType::BROKEN:
    case BarLineType::DOTTED:
    case BarLineType::REVERSE_END:
    case BarLineType::HEAVY:
    case BarLineType::DOUBLE_HEAVY: {
        Segment* segment = bl->segment();
        SegmentType segmentType = segment->segmentType();
        if (segmentType == SegmentType::EndBarLine) {
            // when setting barline type on mmrest, set for underlying measure (and linked staves)
            // createMMRest will then set for the mmrest directly
            Measure* m2 = m->isMMRest() ? m->mmRestLast() : m;

            bool generated;
            if (bl->barLineType() == barType) {
                generated = bl->generated();                // no change: keep current status
            } else if (!bl->generated() && (barType == BarLineType::NORMAL)) {
                generated = true;                           // currently non-generated, changing to normal: assume generated
            } else {
                generated = false;                          // otherwise assume non-generated
            }
            if (allStaves) {
                // use all staves of master score; we will take care of parts in loop through linked staves below
                m2 = bl->masterScore()->tick2measure(m2->tick());
                if (!m2) {
                    return;                     // should never happen
                }
                segment = m2->undoGetSegment(segment->segmentType(), segment->tick());
            }
            const std::vector<Element*>& elist = allStaves ? segment->elist() : std::vector<Element*> { bl };
            for (Element* e : elist) {
                if (!e || !e->staff() || !e->isBarLine()) {
                    continue;
                }

                // handle linked staves/parts:
                // barlines themselves are not necessarily linked,
                // so use staffList to find linked staves
                BarLine* sbl = toBarLine(e);
                for (Staff* lstaff : sbl->staff()->staffList()) {
                    Score* lscore = lstaff->score();
                    int ltrack = lstaff->idx() * VOICES;

                    // handle mmrests:
                    // set the barline on the underlying measure
                    // this will copied to the mmrest during layout, in createMMRest
                    Measure* lmeasure = lscore->tick2measure(m2->tick());
                    if (!lmeasure) {
                        continue;
                    }

                    lmeasure->undoChangeProperty(Pid::REPEAT_END, false);
                    Segment* lsegment = lmeasure->undoGetSegmentR(SegmentType::EndBarLine, lmeasure->ticks());
                    BarLine* lbl = toBarLine(lsegment->element(ltrack));
                    if (!lbl) {
                        lbl = new BarLine(lscore);
                        lbl->setParent(lsegment);
                        lbl->setTrack(ltrack);
                        lbl->setSpanStaff(lstaff->barLineSpan());
                        lbl->setSpanFrom(lstaff->barLineFrom());
                        lbl->setSpanTo(lstaff->barLineTo());
                        lbl->setBarLineType(barType);
                        lbl->setGenerated(generated);
                        lscore->addElement(lbl);
                        if (!generated) {
                            lbl->linkTo(sbl);
                        }
                    } else {
                        lscore->undo(new ChangeProperty(lbl, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                        lscore->undo(new ChangeProperty(lbl, Pid::BARLINE_TYPE, QVariant::fromValue(barType), PropertyFlags::NOSTYLE));
                        // set generated flag before and after so it sticks on type change and also works on undo/redo
                        lscore->undo(new ChangeProperty(lbl, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                        if (lbl != sbl && !generated && !lbl->links()) {
                            lscore->undo(new Link(lbl, sbl));
                        } else if (lbl != sbl && generated && lbl->isLinked(sbl)) {
                            lscore->undo(new Unlink(lbl));
                        }
                    }
                }
            }
        } else if (segmentType == SegmentType::BeginBarLine) {
            Segment* segment1 = m->undoGetSegmentR(SegmentType::BeginBarLine, Fraction(0, 1));
            for (Element* e : segment1->elist()) {
                if (e) {
                    e->score()->undo(new ChangeProperty(e, Pid::GENERATED, false, PropertyFlags::NOSTYLE));
                    e->score()->undo(new ChangeProperty(e, Pid::BARLINE_TYPE, QVariant::fromValue(barType), PropertyFlags::NOSTYLE));
                    // set generated flag before and after so it sticks on type change and also works on undo/redo
                    e->score()->undo(new ChangeProperty(e, Pid::GENERATED, false, PropertyFlags::NOSTYLE));
                }
            }
        }
    }
    break;
    case BarLineType::START_REPEAT: {
        Measure* m2 = m->isMMRest() ? m->mmRestFirst() : m;
        for (int staffIdx = 0; staffIdx < m2->score()->nstaves(); ++staffIdx) {
            if (m2->isMeasureRepeatGroupWithPrevM(staffIdx)) {
                MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                return;
            }
        }
        for (Score* lscore : m2->score()->scoreList()) {
            Measure* lmeasure = lscore->tick2measure(m2->tick());
            if (lmeasure) {
                lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
            }
        }
    }
    break;
    case BarLineType::END_REPEAT: {
        Measure* m2 = m->isMMRest() ? m->mmRestLast() : m;
        for (int staffIdx = 0; staffIdx < m2->score()->nstaves(); ++staffIdx) {
            if (m2->isMeasureRepeatGroupWithNextM(staffIdx)) {
                MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                return;
            }
        }
        for (Score* lscore : m2->score()->scoreList()) {
            Measure* lmeasure = lscore->tick2measure(m2->tick());
            if (lmeasure) {
                lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
            }
        }
    }
    break;
    case BarLineType::END_START_REPEAT: {
        Measure* m2 = m->isMMRest() ? m->mmRestLast() : m;
        for (int staffIdx = 0; staffIdx < m2->score()->nstaves(); ++staffIdx) {
            if (m2->isMeasureRepeatGroupWithNextM(staffIdx)) {
                MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                return;
            }
        }
        for (Score* lscore : m2->score()->scoreList()) {
            Measure* lmeasure = lscore->tick2measure(m2->tick());
            if (lmeasure) {
                lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
                lmeasure = lmeasure->nextMeasure();
                if (lmeasure) {
                    lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
                }
            }
        }
    }
    break;
    }
}

//---------------------------------------------------------
//   BarLineEditData
//---------------------------------------------------------

class BarLineEditData : public ElementEditData
{
public:
    qreal yoff1;
    qreal yoff2;
    virtual EditDataType type() override { return EditDataType::BarLineEditData; }
};

//---------------------------------------------------------
//   BarLineTable
//---------------------------------------------------------

const std::vector<BarLineTableItem> BarLine::barLineTable {
    { BarLineType::NORMAL,           Sym::symUserNames[int(SymId::barlineSingle)],        "normal" },
    { BarLineType::DOUBLE,           Sym::symUserNames[int(SymId::barlineDouble)],        "double" },
    { BarLineType::START_REPEAT,     Sym::symUserNames[int(SymId::repeatLeft)],           "start-repeat" },
    { BarLineType::END_REPEAT,       Sym::symUserNames[int(SymId::repeatRight)],          "end-repeat" },
    { BarLineType::BROKEN,           Sym::symUserNames[int(SymId::barlineDashed)],        "dashed" },
    { BarLineType::END,              Sym::symUserNames[int(SymId::barlineFinal)],         "end" },
    { BarLineType::END_START_REPEAT, Sym::symUserNames[int(SymId::repeatRightLeft)],      "end-start-repeat" },
    { BarLineType::DOTTED,           Sym::symUserNames[int(SymId::barlineDotted)],        "dotted" },
    { BarLineType::REVERSE_END,      Sym::symUserNames[int(SymId::barlineReverseFinal)],  "reverse-end" },
    { BarLineType::HEAVY,            Sym::symUserNames[int(SymId::barlineHeavy)],         "heavy" },
    { BarLineType::DOUBLE_HEAVY,     Sym::symUserNames[int(SymId::barlineHeavyHeavy)],    "double-heavy" },
};

//---------------------------------------------------------
//   barLineTableItem
//---------------------------------------------------------

const BarLineTableItem* BarLine::barLineTableItem(unsigned i)
{
    if (i >= barLineTable.size()) {
        return 0;
    }
    return &barLineTable[i];
}

//---------------------------------------------------------
//   userTypeName
//---------------------------------------------------------

QString BarLine::userTypeName(BarLineType t)
{
    for (const auto& i : barLineTable) {
        if (i.type == t) {
            return qtrc("symUserNames", i.userName);
        }
    }
    return QString();
}

//---------------------------------------------------------
//   barLineTypeName
//
//    Instance form returning the name string of the bar line type and
//    static form returning the name string for an arbitrary bar line type.
//---------------------------------------------------------

QString BarLine::barLineTypeName() const
{
    return barLineTypeName(barLineType());
}

QString BarLine::barLineTypeName(BarLineType t)
{
    for (const auto& i : barLineTable) {
        if (i.type == t) {
            return i.name;
        }
    }
    return QString("??");
}

//---------------------------------------------------------
//   setBarLineType
//
//    Set the bar line type from the type name string.
//    Does not update _customSubtype or _generated flags: to be used when reading from a score file
//---------------------------------------------------------

void BarLine::setBarLineType(const QString& s)
{
    _barLineType = barLineType(s);
}

//---------------------------------------------------------
//   barLineType
//---------------------------------------------------------

BarLineType BarLine::barLineType(const QString& s)
{
    for (const auto& i : barLineTable) {
        if (i.name == s) {
            return i.type;
        }
    }
    return BarLineType::NORMAL;     // silent default
}

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
    : Element(s)
{
    setHeight(4 * spatium());   // for use in palettes
}

BarLine::BarLine(const BarLine& bl)
    : Element(bl)
{
    _spanStaff   = bl._spanStaff;
    _spanFrom    = bl._spanFrom;
    _spanTo      = bl._spanTo;
    _barLineType = bl._barLineType;
    y1           = bl.y1;
    y2           = bl.y2;

    for (Element* e : bl._el) {
        add(e->clone());
    }
}

BarLine::~BarLine()
{
    qDeleteAll(_el);
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF BarLine::canvasPos() const
{
    PointF pos = Element::canvasPos();
    if (parent()) {
        System* system = measure()->system();
        qreal yoff = system ? system->staff(staffIdx())->y() : 0.0;
        pos.ry() += yoff;
    }
    return pos;
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF BarLine::pagePos() const
{
    if (segment() == 0) {
        return pos();
    }
    System* system = segment()->measure()->system();

    qreal yp = y();
    if (system) {
        // get first not hidden staff
        int startIdx        = staffIdx();
        int endIdx          = startIdx + (spanStaff() ? 1 : 0);
        int staffIdx1       = startIdx;
        Staff* staff1       = score()->staff(staffIdx1);
        SysStaff* sysStaff1 = system->staff(staffIdx1);

        while (staff1 && sysStaff1 && !(sysStaff1->show() && staff1->show())) {
            if (++staffIdx1 >= endIdx) {
                // no visible staves spanned; just use first
                staffIdx1 = startIdx;
                break;
            }
            staff1 = score()->staff(staffIdx1);
            sysStaff1 = system->staff(staffIdx1);
        }
        yp += system->staffYpage(staffIdx1);
    }
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   prevVisiblespannedStaff
//---------------------------------------------------------

int prevVisibleSpannedStaff(const BarLine* bl)
{
    Score* score = bl->score();
    int staffIdx = bl->staffIdx();
    Segment* segment = bl->segment();
    for (int i = staffIdx - 1; i >= 0; --i) {
        BarLine* nbl = toBarLine(segment->element(i * VOICES));
        if (!nbl || !nbl->spanStaff()) {
            break;
        }
        Staff* s = score->staff(i);
        if (s->part()->show() && bl->measure()->visible(i)) {
            return i;
        }
    }
    return staffIdx;
}

//---------------------------------------------------------
//   nextVisiblespannedStaff
//---------------------------------------------------------

int nextVisibleSpannedStaff(const BarLine* bl)
{
    Score* score = bl->score();
    int nstaves = score->nstaves();
    int staffIdx = bl->staffIdx();
    Segment* segment = bl->segment();
    for (int i = staffIdx + 1; i < nstaves; ++i) {
        Staff* s = score->staff(i);
        if (s->part()->show()) {
            // span/show bar line if this measure is visible
            if (bl->measure()->visible(i)) {
                return i;
            }
            // or if this is an endBarLine and:
            if (segment && segment->isEndBarLineType()) {
                // ...this measure contains a (cutaway) courtesy clef only
                if (bl->measure()->isCutawayClef(i)) {
                    return i;
                }
                // ...or next measure is both visible and in the same system
                Measure* nm = bl->measure()->nextMeasure();
                if ((nm ? nm->visible(i) : false) && (nm ? nm->system() == bl->measure()->system() : false)) {
                    return i;
                }
            }
        }
        BarLine* nbl = toBarLine(segment->element(i * VOICES));
        if (!nbl || !nbl->spanStaff()) {
            break;
        }
    }
    return staffIdx;
}

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

void BarLine::getY() const
{
    qreal _spatium = spatium();
    if (!parent()) {
        // for use in palette
        y1 = _spanFrom * _spatium * .5;
        y2 = (8 - _spanTo) * _spatium * .5;
        return;
    }
    int staffIdx1       = staffIdx();
    const Staff* staff1 = score()->staff(staffIdx1);
    int staffIdx2       = staffIdx1;
    int nstaves         = score()->nstaves();

    Measure* measure = segment()->measure();
    if (_spanStaff) {
        staffIdx2 = nextVisibleSpannedStaff(this);
    }

    System* system = measure->system();
    if (!system) {
        return;
    }

    // test start and end staff visibility

    // base y on top visible staff in barline span
    // after skipping ones with hideSystemBarLine set
    // and accounting for staves that are shown but have invisible measures

    Fraction tick        = segment()->measure()->tick();
    const StaffType* st1 = staff1->staffType(tick);

    int from    = _spanFrom;
    int to      = _spanTo;
    int oneLine = st1->lines() <= 1;
    if (oneLine && _spanFrom == 0) {
        from = BARLINE_SPAN_1LINESTAFF_FROM;
        if (!_spanStaff || (staffIdx1 == nstaves - 1)) {
            to = BARLINE_SPAN_1LINESTAFF_TO;
        }
    }
    SysStaff* sysStaff1  = system->staff(staffIdx1);
    qreal yp = sysStaff1->y();
    qreal spatium1 = st1->spatium(score());
    qreal d  = st1->lineDistance().val() * spatium1;
    qreal yy = measure->staffLines(staffIdx1)->y1() - yp;
    qreal lw = score()->styleS(Sid::staffLineWidth).val() * spatium1 * .5;
    y1       = yy + from * d * .5 - lw;
    if (staffIdx2 != staffIdx1) {
        y2 = measure->staffLines(staffIdx2)->y1() - yp - to * d * .5;
    } else {
        y2 = yy + (st1->lines() * 2 - 2 + to) * d * .5 + lw;
    }
}

//---------------------------------------------------------
//   drawDots
//---------------------------------------------------------

void BarLine::drawDots(mu::draw::Painter* painter, qreal x) const
{
    qreal _spatium = spatium();

    qreal y1l;
    qreal y2l;
    if (parent() == 0) {      // for use in palette (always Bravura)
        //Bravura shifted repeatDot symbol 0.5sp upper in the font itself (1.272)
        y1l = 1.5 * _spatium;
        y2l = 2.5 * _spatium;
    } else {
        const StaffType* st = staffType();

        //workaround to make new Bravura, Petaluma and Leland font work correctly with repeatDots
        qreal offset
            = (score()->scoreFont()->name() == "Leland" || score()->scoreFont()->name() == "Bravura"
               || score()->scoreFont()->name() == "Petaluma") ? 0 : 0.5 * score()->spatium() * mag();
        y1l          = st->doty1() * _spatium + offset;
        y2l          = st->doty2() * _spatium + offset;

        //adjust for staffType offset
        qreal stYOffset = st->yoffset().val() * _spatium;
        y1l             += stYOffset;
        y2l             += stYOffset;
    }
    drawSymbol(SymId::repeatDot, painter, PointF(x, y1l));
    drawSymbol(SymId::repeatDot, painter, PointF(x, y2l));
}

//---------------------------------------------------------
//   drawTips
//---------------------------------------------------------

void BarLine::drawTips(mu::draw::Painter* painter, bool reversed, qreal x) const
{
    if (reversed) {
        if (isTop()) {
            drawSymbol(SymId::reversedBracketTop, painter, PointF(x - symWidth(SymId::reversedBracketTop), y1));
        }
        if (isBottom()) {
            drawSymbol(SymId::reversedBracketBottom, painter, PointF(x - symWidth(SymId::reversedBracketBottom), y2));
        }
    } else {
        if (isTop()) {
            drawSymbol(SymId::bracketTop, painter, PointF(x, y1));
        }
        if (isBottom()) {
            drawSymbol(SymId::bracketBottom, painter, PointF(x, y2));
        }
    }
}

//---------------------------------------------------------
//   isTop
//---------------------------------------------------------

bool BarLine::isTop() const
{
    int idx = staffIdx();
    if (idx == 0) {
        return true;
    } else {
        return prevVisibleSpannedStaff(this) == idx;
    }
}

//---------------------------------------------------------
//   isBottom
//---------------------------------------------------------

bool BarLine::isBottom() const
{
    if (!_spanStaff) {
        return true;
    }
    int idx = staffIdx();
    if (idx == score()->nstaves() - 1) {
        return true;
    } else {
        return nextVisibleSpannedStaff(this) == idx;
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    switch (barLineType()) {
    case BarLineType::NORMAL: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, y1, lw * .5, y2));
    }
    break;

    case BarLineType::BROKEN: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::DashLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, y1, lw * .5, y2));
    }
    break;

    case BarLineType::DOTTED: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::DotLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, y1, lw * .5, y2));
    }
    break;

    case BarLineType::END: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        qreal x  = lw * .5;
        painter->drawLine(LineF(x, y1, x, y2));

        qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += score()->styleP(Sid::endBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));
    }
    break;

    case BarLineType::DOUBLE: {
        qreal lw2 = score()->styleP(Sid::doubleBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        qreal x = lw2 * .5;
        painter->drawLine(LineF(x, y1, x, y2));
        x += score()->styleP(Sid::doubleBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));
    }
    break;

    case BarLineType::REVERSE_END: {
        qreal lw = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        qreal x = lw * .5;
        painter->drawLine(LineF(x, y1, x, y2));

        qreal lw2 = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += score()->styleP(Sid::endBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));
    }
    break;

    case BarLineType::HEAVY: {
        qreal lw = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, y1, lw * .5, y2));
    }
    break;

    case BarLineType::DOUBLE_HEAVY: {
        qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        qreal x = lw2 * .5;
        painter->drawLine(LineF(x, y1, x, y2));
        x += score()->styleP(Sid::endBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));
    }
    break;

    case BarLineType::START_REPEAT: {
        qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        qreal x = lw2 * .5;
        painter->drawLine(LineF(x, y1, x, y2));

        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += score()->styleP(Sid::endBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));

        x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
        x -= symBbox(SymId::repeatDot).width() * .5;
        drawDots(painter, x);

        if (score()->styleB(Sid::repeatBarTips)) {
            drawTips(painter, false, 0.0);
        }
    }
    break;

    case BarLineType::END_REPEAT: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        qreal x = 0.0;           // symBbox(SymId::repeatDot).width() * .5;
        drawDots(painter, x);

        x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
        x += symBbox(SymId::repeatDot).width() * .5;
        painter->drawLine(LineF(x, y1, x, y2));

        x  += score()->styleP(Sid::endBarDistance) * mag();

        qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, y1, x, y2));

        if (score()->styleB(Sid::repeatBarTips)) {
            drawTips(painter, true, x + lw2 * .5);
        }
    }
    break;
    case BarLineType::END_START_REPEAT: {
        qreal lw = score()->styleP(Sid::barWidth) * mag();
        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        qreal x = 0.0;           // symBbox(SymId::repeatDot).width() * .5;
        drawDots(painter, x);

        x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
        x += symBbox(SymId::repeatDot).width() * .5;
        painter->drawLine(LineF(x, y1, x, y2));

        x  += score()->styleP(Sid::endBarDistance) * mag();

        qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
        painter->setPen(Pen(curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, y1, x, y2));

        if (score()->styleB(Sid::repeatBarTips)) {
            drawTips(painter, true, x + lw2 * .5);
        }

        painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += score()->styleP(Sid::endBarDistance) * mag();
        painter->drawLine(LineF(x, y1, x, y2));

        x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
        x -= symBbox(SymId::repeatDot).width() * .5;
        drawDots(painter, x);

        if (score()->styleB(Sid::repeatBarTips)) {
            drawTips(painter, false, 0.0);
        }
    }
    break;
    }
    Segment* s = segment();
    if (s && s->isEndBarLineType() && !score()->printing() && score()->showUnprintable()) {
        Measure* m = s->measure();
        if (m->isIrregular() && score()->markIrregularMeasures() && !m->isMMRest()) {
            painter->setPen(MScore::layoutBreakColor);
            mu::draw::Font f("Edwin");
            f.setPointSizeF(12 * spatium() * MScore::pixelRatio / SPATIUM20);
            f.setBold(true);
            QString str = m->ticks() > m->timesig() ? "+" : "-";
            RectF r = mu::draw::FontMetrics(f).boundingRect(str);
            painter->setFont(f);
            painter->drawText(-r.width(), 0.0, str);
        }
    }
}

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void BarLine::drawEditMode(mu::draw::Painter* p, EditData& ed)
{
    Element::drawEditMode(p, ed);
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));
    y1 += bed->yoff1;
    y2 += bed->yoff2;
    PointF pos(canvasPos());
    p->translate(pos);
    BarLine::draw(p);
    p->translate(-pos);
    y1 -= bed->yoff1;
    y2 -= bed->yoff2;
}

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction BarLine::playTick() const
{
    // Play from the start of the measure to the right of the barline, unless this is the last barline in either the entire score or the system,
    // in which case we should play from the start of the measure to the left of the barline.
    const auto measure = findMeasure();
    if (measure) {
        const auto nextMeasure = findMeasure()->next();
        if (!nextMeasure || (nextMeasure->system() != measure->system())) {
            return measure->tick();
        }
    }

    return tick();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(XmlWriter& xml) const
{
    xml.stag(this);

    writeProperty(xml, Pid::BARLINE_TYPE);
    writeProperty(xml, Pid::BARLINE_SPAN);
    writeProperty(xml, Pid::BARLINE_SPAN_FROM);
    writeProperty(xml, Pid::BARLINE_SPAN_TO);

    for (const Element* e : _el) {
        e->write(xml);
    }
    Element::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(XmlReader& e)
{
    resetProperty(Pid::BARLINE_SPAN);
    resetProperty(Pid::BARLINE_SPAN_FROM);
    resetProperty(Pid::BARLINE_SPAN_TO);

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            setBarLineType(e.readElementText());
        } else if (tag == "span") {
            _spanStaff  = e.readBool();
        } else if (tag == "spanFromOffset") {
            _spanFrom = e.readInt();
        } else if (tag == "spanToOffset") {
            _spanTo = e.readInt();
        } else if (tag == "Articulation") {
            Articulation* a = new Articulation(score());
            a->read(e);
            add(a);
        } else if (tag == "Symbol") {
            Symbol* s = new Symbol(score());
            s->setTrack(track());
            s->read(e);
            add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(score());
                image->setTrack(track());
                image->read(e);
                add(image);
            }
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(EditData& data) const
{
    ElementType type = data.dropElement->type();
    if (type == ElementType::BAR_LINE) {
        return true;
    } else {
        return (type == ElementType::ARTICULATION || type == ElementType::FERMATA || type == ElementType::SYMBOL
                || type == ElementType::IMAGE)
               && segment()
               && segment()->isEndBarLineType();
    }
    // Prevent unreachable code warning
    // return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(EditData& data)
{
    Element* e = data.dropElement;

    if (e->isBarLine()) {
        BarLine* bl    = toBarLine(e);
        BarLineType st = bl->barLineType();

        // if no change in subtype or no change in span, do nothing
        if (st == barLineType() && !bl->spanFrom() && !bl->spanTo()) {
            delete e;
            return 0;
        }

        // check if the new property can apply to this single bar line
        BarLineType bt = BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT;
        bool oldRepeat = barLineType() & bt;
        bool newRepeat = bl->barLineType() & bt;

        // if ctrl was used and repeats are not involved,
        // or if drop refers to span rather than subtype =>
        // single bar line drop

        if ((data.control() && !oldRepeat && !newRepeat) || (bl->spanFrom() || bl->spanTo())) {
            // if drop refers to span, update this bar line span
            if (bl->spanFrom() || bl->spanTo()) {
                // if dropped spanFrom or spanTo are below the middle of standard staff (5 lines)
                // adjust to the number of staff lines
                // TODO:barlines
                int spanFrom   = bl->spanFrom();
                int spanTo     = bl->spanTo();
                undoChangeProperty(Pid::BARLINE_SPAN, false);
                undoChangeProperty(Pid::BARLINE_SPAN_FROM, spanFrom);
                undoChangeProperty(Pid::BARLINE_SPAN_TO, spanTo);
            }
            // if drop refers to subtype, update this bar line subtype
            else {
                undoChangeBarLineType(this, st, false);
            }
        } else {
            undoChangeBarLineType(this, st, true);
        }
        delete e;
    } else if (e->isArticulation()) {
        Articulation* atr = toArticulation(e);
        atr->setParent(this);
        atr->setTrack(track());
        score()->undoAddElement(atr);
        return atr;
    } else if (e->isSymbol() || e->isImage()) {
        e->setParent(this);
        e->setTrack(track());
        score()->undoAddElement(e);
        return e;
    } else if (e->isFermata()) {
        e->setPlacement(track() & 1 ? Placement::BELOW : Placement::ABOVE);
        for (Element* el: segment()->annotations()) {
            if (el->isFermata() && (el->track() == track())) {
                if (el->subtype() == e->subtype()) {
                    delete e;
                    return el;
                } else {
                    e->setPlacement(el->placement());
                    e->setTrack(track());
                    e->setParent(segment());
                    score()->undoChangeElement(el, e);
                    return e;
                }
            }
        }
        e->setTrack(track());
        e->setParent(segment());
        score()->undoAddElement(e);
        return e;
    }
    return 0;
}

//---------------------------------------------------------
//   setShowTips
//---------------------------------------------------------

void BarLine::setShowTips(bool val)
{
    if (!score()) {
        return;
    }

    score()->undoChangeStyleVal(Sid::repeatBarTips, val);
}

//---------------------------------------------------------
//   showTips
//---------------------------------------------------------

bool BarLine::showTips() const
{
    if (!score()) {
        return false;
    }

    return score()->styleB(Sid::repeatBarTips);
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> BarLine::gripsPositions(const EditData& ed) const
{
    const BarLineEditData* bed = static_cast<const BarLineEditData*>(ed.getData(this));

    qreal lw = score()->styleP(Sid::barWidth) * staff()->staffMag(tick());
    getY();

    const PointF pp = pagePos();

    return {
        PointF(lw * .5, y1 + bed->yoff1) + pp,
        PointF(lw * .5, y2 + bed->yoff2) + pp
    };
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void BarLine::startEdit(EditData& ed)
{
    BarLineEditData* bed = new BarLineEditData();
    bed->e     = this;
    bed->yoff1 = 0;
    bed->yoff2 = 0;
    ed.addData(bed);
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit(EditData&)
{
#if 0 // TODO
    if (ctrlDrag) {                             // if single bar line edit
        char newSpanStaff = _spanStaff;              // copy edited span values
        char newSpanFrom  = _spanFrom;
        char newSpanTo    = _spanTo;
        _spanStaff        = _origSpanStaff;          // restore original span values
        _spanFrom         = _origSpanFrom;
        _spanTo           = _origSpanTo;
        // for mid-measure barline in root score, update parts
        if (midMeasure && score()->isMaster() && score()->excerpts().size() > 0) {
            int currIdx = staffIdx();
            Measure* m = segment()->measure();
            // change linked barlines as necessary
            int lastIdx = currIdx + qMax(_span, newSpanStaff);
            for (int idx = currIdx; idx < lastIdx; ++idx) {
                Staff* staff = score()->staff(idx);
                LinkedStaves* ls = staff->linkedStaves();
                if (ls) {
                    for (Staff* lstaff : ls->staves()) {
                        Score* lscore = lstaff->score();
                        // don't change barlines in root score
                        if (lscore == staff->score()) {
                            continue;
                        }
                        // change barline only in top staff of part
                        if (lstaff != lscore->staff(0)) {
                            continue;
                        }
                        int spannedStaves = qMax(currIdx + newSpanStaff - idx, 0);
                        int lNewSpan = qMin(spannedStaves, lscore->nstaves());
                        Measure* lm = lscore->tick2measure(m->tick());
                        Segment* lseg = lm->undoGetSegmentR(SegmentType::BarLine, rtick());
                        BarLine* lbl = toBarLine(lseg->element(0));
                        if (lbl) {
                            // already a barline here
                            if (lNewSpan > 0) {
                                // keep barline, but update span if necessary
                                if (lbl->span() != lNewSpan) {
                                    lbl->undoChangeProperty(Pid::BARLINE_SPAN, lNewSpan);
                                }
                            } else {
                                // remove barline
                                lbl->unlink();
                                lbl->score()->undoRemoveElement(lbl);
                            }
                        } else {
                            // new barline needed
                            lbl = toBarLine(linkedClone());
                            lbl->setSpan(lNewSpan);
                            lbl->setTrack(lstaff->idx() * VOICES);
                            lbl->setScore(lscore);
                            lbl->setParent(lseg);
                            lscore->undoAddElement(lbl);
                        }
                    }
                }
            }
        }
        undoChangeProperty(Pid::BARLINE_SPAN,      newSpanStaff);
        undoChangeProperty(Pid::BARLINE_SPAN_FROM, newSpanFrom);
        undoChangeProperty(Pid::BARLINE_SPAN_FROM, newSpanTo);
        return;
    }

    // if same as staff settings, do nothing
    if (staff()->barLineSpan() == _spanStaff && staff()->barLineFrom() == _spanFrom && staff()->barLineTo() == _spanTo) {
        return;
    }

//      int idx1 = staffIdx();
    if (_span != staff()->barLineSpan()) {
        // if now bar lines span more staves
        if (_span > staff()->barLineSpan()) {
            int idx2 = idx1 + _span;
            // set span 0 to all additional staves
            for (int idx = idx1 + 1; idx < idx2; ++idx) {
                // Mensurstrich special case:
                // if line spans to top line of a stave AND current staff is
                //    the last spanned staff BUT NOT the last score staff
                //          keep its bar lines
                // otherwise remove them
                if (_spanTo > 0 || !(idx == idx2 - 1 && idx != score()->nstaves() - 1)) {
                    Staff* staff = score()->staff(idx);
                    staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      0);
                    staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, 0);
                    staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   (staff->lines(tick()) - 1) * 2);
                }
            }
        }
        // if now bar lines span fewer staves
        else {
            int idx1 = staffIdx() + _span;
            int idx2 = staffIdx() + staff()->barLineSpan();
            // set standard span for each no-longer-spanned staff
            for (int idx = idx1; idx < idx2; ++idx) {
                Staff* staff = score()->staff(idx);
                int lines = staff->lines(tick());
                int spanFrom = 0;
                int spanTo   = 0;
                staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      1);
                staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, spanFrom);
                staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   spanTo);
            }
        }
    }
    // update span for the staff the edited bar line belongs to
    staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      _spanStaff);
    staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, _spanFrom);
    staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   _spanTo);
#endif
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(EditData& ed)
{
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));

    qreal lineDist = staff()->lineDistance(tick()) * spatium();
    getY();
    if (ed.curGrip == Grip::START) {
        // Start grip moving is useless currently, so disable it
        return;
#if 0
        // min offset for top grip is line -1 (-2 for 1-line staves)
        // max offset is 1 line above bottom grip or 1 below last staff line, whichever comes first
        int lines = staff()->lines(tick());
        qreal min = (-y1 - lines == 1) ? lineDist * 2 : lineDist;
        qreal max = y2 - y1 - lineDist;                                       // 1 line above bottom grip
        qreal lastmax = (lines - _spanFrom / 2) * lineDist;        // 1 line below last staff line
        if (lastmax < max) {
            max = lastmax;
        }
        // update yoff1 and bring it within limits
        bed->yoff1 += ed.delta.y();
        if (bed->yoff1 < min) {
            bed->yoff1 = min;
        }
        if (bed->yoff1 > max) {
            bed->yoff1 = max;
        }
#endif
    } else {
        // min for bottom grip is 1 line below top grip
        const qreal min = y1 - y2 + lineDist;
        // max is the bottom of the system
        const System* system = segment() ? segment()->system() : nullptr;
        const int st = staffIdx();
        const qreal max = (system && st != -1) ? (system->height() - y2 - system->staff(st)->y()) : std::numeric_limits<qreal>::max();
        // update yoff2 and bring it within limit
        bed->yoff2 += ed.delta.y();
        if (bed->yoff2 < min) {
            bed->yoff2 = min;
        }
        if (bed->yoff2 > max) {
            bed->yoff2 = max;
        }
    }
}

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff / staff line
//---------------------------------------------------------

void BarLine::endEditDrag(EditData& ed)
{
    getY();
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));
    y1 += bed->yoff1;
    y2 += bed->yoff2;

    qreal ay0      = pagePos().y();
    qreal ay2      = ay0 + y2;                       // absolute (page-relative) bar line bottom coord
    int staffIdx1  = staffIdx();
    System* syst   = segment()->measure()->system();
    qreal systTopY = syst->pagePos().y();

    // determine new span value
    int staffIdx2;
    int numOfStaves = syst->staves()->size();
    if (staffIdx1 + 1 >= numOfStaves) {
        // if initial staff is last staff, ending staff must be the same
        staffIdx2 = staffIdx1;
    } else {
        // if there are other staves after it, look for staff nearest to bar line bottom coord
        qreal staff1TopY = syst->staff(staffIdx1)->y() + systTopY;

        for (staffIdx2 = staffIdx1 + 1; staffIdx2 < numOfStaves; ++staffIdx2) {
            // compute 1st staff height, absolute top Y of 2nd staff and height of blank between the staves
            Staff* staff1      = score()->staff(staffIdx2 - 1);
            qreal staff1Hght    = (staff1->lines(tick()) - 1) * staff1->lineDistance(tick()) * spatium();
            qreal staff2TopY    = systTopY + syst->staff(staffIdx2)->y();
            qreal blnkBtwnStaff = staff2TopY - staff1TopY - staff1Hght;
            // if bar line bottom coord is above than mid-way of blank between staves...
            if (ay2 < (staff1TopY + staff1Hght + blnkBtwnStaff * .5)) {
                break;                          // ...staff 1 is ending staff
            }
            // if bar line is below, advance to next staff
            staff1TopY = staff2TopY;
        }
        staffIdx2 -= 1;
    }

    // determine new spanFrom and spanTo values
    int newSpanFrom, newSpanTo;
//      Staff * staff2    = score()->staff(staffIdx2);
//      int staff1lines   = staff()->lines(tick());
//      int staff2lines   = staff2->lines(tick());

#if 0       // TODO
    if (shiftDrag) {                      // if precision dragging
        newSpanFrom = _spanFrom;
        if (yoff1 != 0.0) {
            // round bar line top coord to nearest line of 1st staff (in half line dist units)
            newSpanFrom = ((int)floor(y1 / (staff()->lineDistance(tick()) * spatium()) + 0.5)) * 2;
            // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
            // except for 1-line staves
            int minFrom = staff1lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (newSpanFrom < minFrom) {
                newSpanFrom = minFrom;
            }
            if (newSpanFrom > staff1lines * 2) {
                newSpanFrom = staff1lines * 2;
            }
        }

        newSpanTo = _spanTo;
        if (yoff2 != 0.0) {
            // round bar line bottom coord to nearest line of 2nd staff (in half line dist units)
            qreal staff2TopY = systTopY + syst->staff(staffIdx2)->y();
            newSpanTo = ((int)floor((ay2 - staff2TopY) / (staff2->lineDistance(tick()) * spatium()) + 0.5)) * 2;
            // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
            int maxTo = staff2lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : staff2lines * 2;
            if (newSpanTo < MIN_BARLINE_SPAN_FROMTO) {
                newSpanTo = MIN_BARLINE_SPAN_FROMTO;
            }
            if (newSpanTo > maxTo) {
                newSpanTo = maxTo;
            }
        }
    }
#endif
//      else {                              // if coarse dragging
    {                                   // if coarse dragging
        newSpanFrom = 0;
        newSpanTo   = 0;
    }

    bool localDrag = ed.control() || segment()->isBarLineType();
    if (localDrag) {
        Segment* s = segment();
        for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
            BarLine* b = toBarLine(s->element(staffIdx * VOICES));
            if (!b) {
                b = toBarLine(linkedClone());
                b->setSpanStaff(true);
                b->setTrack(staffIdx * VOICES);
                b->setScore(score());
                b->setParent(s);
                score()->undoAddElement(b);
            }
            b->undoChangeProperty(Pid::BARLINE_SPAN, true);
        }
        BarLine* b = toBarLine(s->element(staffIdx2 * VOICES));
        if (b) {
            b->undoChangeProperty(Pid::BARLINE_SPAN, false);
        }
    } else {
        for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
            score()->staff(staffIdx)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, true);
        }
        score()->staff(staffIdx2)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
        staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, newSpanFrom);
        staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   newSpanTo);
    }

    bed->yoff1 = 0.0;
    bed->yoff2 = 0.0;
}

//---------------------------------------------------------
//   layoutWidth
//---------------------------------------------------------

qreal BarLine::layoutWidth(Score* score, BarLineType type)
{
    qreal dotwidth = score->scoreFont()->width(SymId::repeatDot, 1.0);

    qreal w { 0.0 };
    switch (type) {
    case BarLineType::DOUBLE:
        w = score->styleP(Sid::doubleBarWidth) + score->styleP(Sid::doubleBarDistance);
        break;
    case BarLineType::DOUBLE_HEAVY:
        w = score->styleP(Sid::endBarWidth) + score->styleP(Sid::endBarDistance);
        break;
    case BarLineType::END_START_REPEAT:
        w = score->styleP(Sid::endBarDistance) * 2
            + score->styleP(Sid::repeatBarlineDotSeparation) * 2
            + dotwidth;
        break;
    case BarLineType::START_REPEAT:
    case BarLineType::END_REPEAT:
        w = score->styleP(Sid::endBarWidth) * .5
            + score->styleP(Sid::endBarDistance)
            + score->styleP(Sid::repeatBarlineDotSeparation)
            + dotwidth * .5;
        break;
    case BarLineType::END:
    case BarLineType::REVERSE_END:
        w = (score->styleP(Sid::endBarWidth) + score->styleP(Sid::barWidth)) * .5
            + score->styleP(Sid::endBarDistance);
        break;
    case BarLineType::BROKEN:
    case BarLineType::NORMAL:
    case BarLineType::DOTTED:
        w = score->styleP(Sid::barWidth);
        break;
    case BarLineType::HEAVY:
        w = score->styleP(Sid::endBarWidth);
        break;
    }
    return w;
}

//---------------------------------------------------------
//   layoutRect
//---------------------------------------------------------

RectF BarLine::layoutRect() const
{
    RectF bb = bbox();
    if (staff()) {
        // actual height may include span to next staff
        // but this should not be included in shapes or skylines
        qreal sp = spatium();
        int span = staff()->lines(tick()) - 1;
        int sFrom;
        int sTo;
        if (span == 0 && _spanTo == 0) {
            sFrom = BARLINE_SPAN_1LINESTAFF_FROM;
            sTo = _spanStaff ? 0 : BARLINE_SPAN_1LINESTAFF_TO;
        } else {
            sFrom = _spanFrom;
            sTo = _spanStaff ? 0 : _spanTo;
        }
        qreal y = sp * sFrom * 0.5;
        qreal h = sp * (span + (sTo - sFrom) * 0.5);
        if (score()->styleB(Sid::repeatBarTips)) {
            switch (barLineType()) {
            case BarLineType::START_REPEAT:
            case BarLineType::END_REPEAT:
            case BarLineType::END_START_REPEAT: {
                if (isTop()) {
                    qreal top = symBbox(SymId::bracketTop).height();
                    y -= top;
                    h += top;
                }
                if (isBottom()) {
                    qreal bottom = symBbox(SymId::bracketBottom).height();
                    h += bottom;
                }
            }
            default:
                break;
            }
        }
        bb.setY(y);
        bb.setHeight(h);
    }
    return bb;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BarLine::layout()
{
    setPos(PointF());
    // barlines hidden on this staff
    if (staff() && segment()) {
        if ((!staff()->staffTypeForElement(this)->showBarlines() && segment()->segmentType() == SegmentType::EndBarLine)
            || (staff()->hideSystemBarLine() && segment()->segmentType() == SegmentType::BeginBarLine)) {
            setbbox(RectF());
            return;
        }
    }

    setMag(score()->styleB(Sid::scaleBarlines) && staff() ? staff()->staffMag(tick()) : 1.0);
    qreal _spatium = spatium();
    y1 = _spatium * .5 * _spanFrom;
    y2 = _spatium * .5 * (8.0 + _spanTo);

    qreal w = layoutWidth(score(), barLineType()) * mag();
    RectF r(0.0, y1, w, y2 - y1);

    if (score()->styleB(Sid::repeatBarTips)) {
        switch (barLineType()) {
        case BarLineType::START_REPEAT:
            r.unite(symBbox(SymId::bracketTop).translated(0, y1));
            // r |= symBbox(SymId::bracketBottom).translated(0, y2);
            break;
        case BarLineType::END_REPEAT: {
            qreal w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(symBbox(SymId::reversedBracketTop).translated(-w1, y1));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        case BarLineType::END_START_REPEAT: {
            qreal w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(symBbox(SymId::reversedBracketTop).translated(-w1, y1));
            r.unite(symBbox(SymId::bracketTop).translated(0, y1));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        default:
            break;
        }
    }
    setbbox(r);

    for (Element* e : _el) {
        e->layout();
        if (e->isArticulation()) {
            Articulation* a  = toArticulation(e);
            Direction dir    = a->direction();
            qreal distance   = 0.5 * spatium();
            qreal x          = width() * .5;
            if (dir == Direction::DOWN) {
                qreal botY = y2 + distance;
                a->setPos(PointF(x, botY));
            } else {
                qreal topY = y1 - distance;
                a->setPos(PointF(x, topY));
            }
        }
    }
}

//---------------------------------------------------------
//   layout2
//    called after system layout; set vertical dimensions
//---------------------------------------------------------

void BarLine::layout2()
{
    // barlines hidden on this staff
    if (staff() && segment()) {
        if ((!staff()->staffTypeForElement(this)->showBarlines() && segment()->segmentType() == SegmentType::EndBarLine)
            || (staff()->hideSystemBarLine() && segment()->segmentType() == SegmentType::BeginBarLine)) {
            setbbox(RectF());
            return;
        }
    }

    getY();
    bbox().setTop(y1);
    bbox().setBottom(y2);

    if (score()->styleB(Sid::repeatBarTips)) {
        switch (barLineType()) {
        case BarLineType::START_REPEAT:
            bbox().unite(symBbox(SymId::bracketTop).translated(0, y1));
            bbox().unite(symBbox(SymId::bracketBottom).translated(0, y2));
            break;
        case BarLineType::END_REPEAT:
        {
            qreal w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            bbox().unite(symBbox(SymId::reversedBracketTop).translated(-w1, y1));
            bbox().unite(symBbox(SymId::reversedBracketBottom).translated(-w1, y2));
            break;
        }
        case BarLineType::END_START_REPEAT:
        {
            qreal w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            bbox().unite(symBbox(SymId::reversedBracketTop).translated(-w1, y1));
            bbox().unite(symBbox(SymId::reversedBracketBottom).translated(-w1, y2));
            bbox().unite(symBbox(SymId::bracketTop).translated(0, y1));
            bbox().unite(symBbox(SymId::bracketBottom).translated(0, y2));
            break;
        }
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape BarLine::shape() const
{
    Shape shape;
#ifndef NDEBUG
    shape.add(bbox(), name());
#else
    shape.add(bbox());
#endif
    return shape;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BarLine::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    // if no width (staff has bar lines turned off) and not all requested, do nothing
    if (width() == 0.0 && !all) {
        return;
    }
    ScoreElement::scanElements(data, func, all);
    func(data, this);
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void BarLine::setTrack(int t)
{
    Element::setTrack(t);
    for (Element* e : _el) {
        e->setTrack(t);
    }
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void BarLine::setScore(Score* s)
{
    Element::setScore(s);
    for (Element* e : _el) {
        e->setScore(s);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BarLine::add(Element* e)
{
    e->setParent(this);
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        _el.push_back(e);
        setGenerated(false);
        break;
    default:
        qDebug("BarLine::add() not impl. %s", e->name());
        delete e;
        break;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BarLine::remove(Element* e)
{
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        if (!_el.remove(e)) {
            qDebug("BarLine::remove(): cannot find %s", e->name());
        }
        break;
    default:
        qDebug("BarLine::remove() not impl. %s", e->name());
        break;
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(Pid id) const
{
    switch (id) {
    case Pid::BARLINE_TYPE:
        return QVariant::fromValue(_barLineType);
    case Pid::BARLINE_SPAN:
        return spanStaff();
    case Pid::BARLINE_SPAN_FROM:
        return int(spanFrom());
    case Pid::BARLINE_SPAN_TO:
        return int(spanTo());
    case Pid::BARLINE_SHOW_TIPS:
        return showTips();
    default:
        break;
    }
    return Element::getProperty(id);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(Pid id, const QVariant& v)
{
    switch (id) {
    case Pid::BARLINE_TYPE:
        setBarLineType(v.value<BarLineType>());
        break;
    case Pid::BARLINE_SPAN:
        setSpanStaff(v.toBool());
        break;
    case Pid::BARLINE_SPAN_FROM:
        setSpanFrom(v.toInt());
        break;
    case Pid::BARLINE_SPAN_TO:
        setSpanTo(v.toInt());
        break;
    case Pid::BARLINE_SHOW_TIPS:
        setShowTips(v.toBool());
        break;
    default:
        return Element::setProperty(id, v);
    }
    setGenerated(false);
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void BarLine::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
{
    if (id == Pid::BARLINE_TYPE && segment()) {
        const BarLine* bl = this;
        BarLineType blType = v.value<BarLineType>();
        if (blType == BarLineType::START_REPEAT) {     // change next measures endBarLine
            if (bl->measure()->nextMeasure()) {
                bl = bl->measure()->nextMeasure()->endBarLine();
            } else {
                bl = 0;
            }
        }
        if (bl) {
            undoChangeBarLineType(const_cast<BarLine*>(bl), v.value<BarLineType>(), true);
        }
    } else {
        ScoreElement::undoChangeProperty(id, v, ps);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant BarLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BARLINE_TYPE:
// dynamic default values are a bad idea: writing to xml the value maybe ommited resulting in
//    wrong values on read (as the default may be different on read)
//                  if (segment() && segment()->measure() && !segment()->measure()->nextMeasure())
//                        return QVariant::fromValue(BarLineType::END);
        return QVariant::fromValue(BarLineType::NORMAL);

    case Pid::BARLINE_SPAN:
        return staff() ? staff()->barLineSpan() : false;

    case Pid::BARLINE_SPAN_FROM:
        return staff() ? staff()->barLineFrom() : 0;

    case Pid::BARLINE_SPAN_TO:
        return staff() ? staff()->barLineTo() : 0;

    case Pid::BARLINE_SHOW_TIPS:
        return false;
    default:
        break;
    }
    return Element::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid BarLine::propertyId(const QStringRef& name) const
{
    if (name == "subtype") {
        return Pid::BARLINE_TYPE;
    }
    return Element::propertyId(name);
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* BarLine::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());      //score()->inputState().prevTrack() / VOICES);
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* BarLine::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());       //score()->inputState().prevTrack() / VOICES);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString BarLine::accessibleInfo() const
{
    return QString("%1: %2").arg(Element::accessibleInfo(), BarLine::userTypeName(barLineType()));
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString BarLine::accessibleExtraInfo() const
{
    Segment* seg = segment();
    QString rez;

    for (const Element* e : *el()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
    }

    for (const Element* e : seg->annotations()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        if (e->track() == track()) {
            rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
        }
    }
    Measure* m = seg->measure();

    if (m) {      // always true?
        //jumps
        for (const Element* e : m->el()) {
            if (!score()->selectionFilter().canSelect(e)) {
                continue;
            }
            if (e->type() == ElementType::JUMP) {
                rez= QString("%1 %2").arg(rez, e->screenReaderInfo());
            }
            if (e->type() == ElementType::MARKER) {
                const Marker* m1 = toMarker(e);
                if (m1->markerType() == Marker::Type::FINE) {
                    rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
                }
            }
        }
        //markers
        Measure* nextM = m->nextMeasureMM();
        if (nextM) {
            for (const Element* e : nextM->el()) {
                if (!score()->selectionFilter().canSelect(e)) {
                    continue;
                }
                if (e->isMarker()) {
                    if (toMarker(e)->markerType() == Marker::Type::FINE) {
                        continue;             //added above^
                    }
                    rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
                }
            }
        }
    }

    Fraction tick = seg->tick();

    auto spanners = score()->spannerMap().findOverlapping(tick.ticks(), tick.ticks());
    for (auto interval : spanners) {
        Spanner* s = interval.value;
        if (!score()->selectionFilter().canSelect(s)) {
            continue;
        }
        if (s->type() == ElementType::VOLTA) {
            if (s->tick() == tick) {
                rez = QObject::tr("%1 Start of %2").arg(rez, s->screenReaderInfo());
            }
            if (s->tick2() == tick) {
                rez = QObject::tr("%1 End of %2").arg(rez, s->screenReaderInfo());
            }
        }
    }
    return rez;
}
}
