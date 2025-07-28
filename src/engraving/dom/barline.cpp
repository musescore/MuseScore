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

#include "barline.h"

#include "dom/text.h"
#include "translation.h"
#include "types/symnames.h"

#include "articulation.h"
#include "factory.h"
#include "marker.h"
#include "masterscore.h"
#include "measure.h"
#include "part.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "staff.h"
#include "stafflines.h"
#include "stafftype.h"
#include "system.h"
#include "undo.h"
#include "types/typesconv.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::read400;

static ElementStyle playCountStyle {
    { Sid::repeatPlayCountPlacement, Pid::PLACEMENT },
    { Sid::repeatPlayCountMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   BarLineTable
//---------------------------------------------------------

const std::vector<BarLineTableItem> BarLine::barLineTable {
    { BarLineType::NORMAL,           SymNames::userNameForSymId(SymId::barlineSingle) },
    { BarLineType::DOUBLE,           SymNames::userNameForSymId(SymId::barlineDouble) },
    { BarLineType::START_REPEAT,     SymNames::userNameForSymId(SymId::repeatLeft) },
    { BarLineType::END_REPEAT,       SymNames::userNameForSymId(SymId::repeatRight) },
    { BarLineType::BROKEN,           SymNames::userNameForSymId(SymId::barlineDashed) },
    { BarLineType::END,              SymNames::userNameForSymId(SymId::barlineFinal) },
    { BarLineType::END_START_REPEAT, SymNames::userNameForSymId(SymId::repeatRightLeft) },
    { BarLineType::DOTTED,           SymNames::userNameForSymId(SymId::barlineDotted) },
    { BarLineType::REVERSE_END,      SymNames::userNameForSymId(SymId::barlineReverseFinal) },
    { BarLineType::HEAVY,            SymNames::userNameForSymId(SymId::barlineHeavy) },
    { BarLineType::DOUBLE_HEAVY,     SymNames::userNameForSymId(SymId::barlineHeavyHeavy) },
};

//---------------------------------------------------------
//   translatedUserTypeName
//---------------------------------------------------------

String BarLine::translatedUserTypeName(BarLineType t)
{
    for (const auto& i : barLineTable) {
        if (i.type == t) {
            return i.userName.translated();
        }
    }
    return String();
}

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Segment* parent)
    : EngravingItem(ElementType::BAR_LINE, parent)
{
    setHeight(4 * spatium());   // for use in palettes
}

BarLine::BarLine(const BarLine& bl)
    : EngravingItem(bl)
{
    m_spanStaff   = bl.m_spanStaff;
    m_spanFrom    = bl.m_spanFrom;
    m_spanTo      = bl.m_spanTo;
    m_barLineType = bl.m_barLineType;
    m_playCount   = bl.m_playCount;

    for (EngravingItem* e : bl.m_el) {
        add(e->clone());
    }
}

BarLine::~BarLine()
{
    muse::DeleteAll(m_el);
}

void BarLine::setParent(Segment* parent)
{
    EngravingItem::setParent(parent);
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF BarLine::canvasPos() const
{
    PointF pos = EngravingItem::canvasPos();
    if (explicitParent()) {
        System* system = measure()->system();
        double yoff = system ? system->staff(staffIdx())->y() : 0.0;
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

    double yp = y();
    if (system) {
        // get first not hidden staff
        staff_idx_t startIdx = staffIdx();
        staff_idx_t endIdx = startIdx + (spanStaff() ? 1 : 0);
        staff_idx_t staffIdx1 = startIdx;
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

static staff_idx_t prevVisibleSpannedStaff(const BarLine* bl)
{
    Score* score = bl->score();
    int staffIdx = static_cast<int>(bl->staffIdx());
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

static size_t nextVisibleSpannedStaff(const BarLine* bl)
{
    Score* score = bl->score();
    size_t nstaves = score->nstaves();
    size_t staffIdx = bl->staffIdx();
    Segment* segment = bl->segment();
    for (size_t i = staffIdx + 1; i < nstaves; ++i) {
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
//   calcY
//---------------------------------------------------------

void BarLine::calcY()
{
    BarLine::LayoutData* data = mutldata();
    double _spatium = spatium();
    if (!explicitParent()) {
        // for use in palette
        data->y1 = m_spanFrom * _spatium * .5;
        data->y2 = (8 - m_spanTo) * _spatium * .5;
        return;
    }
    staff_idx_t staffIdx1 = staffIdx();
    staff_idx_t staffIdx2 = m_spanStaff ? nextVisibleSpannedStaff(this) : staffIdx1;

    bool spanStaff = staffIdx2 != staffIdx1;

    Measure* measure = segment()->measure();
    System* system = measure->system();
    if (!system) {
        return;
    }

    Fraction tick = segment()->measure()->tick();
    const Staff* staff1 = score()->staff(staffIdx1);
    const StaffType* staffType1 = staff1->staffType(tick);

    bool oneLine = staffType1->lines() <= 1;

    int from = m_spanFrom;
    int to = m_spanTo;

    if (oneLine && m_spanFrom == 0 && m_spanTo == 0) {
        from = BARLINE_SPAN_1LINESTAFF_FROM;
        if (!spanStaff) {
            to = BARLINE_SPAN_1LINESTAFF_TO;
        }
    }

    double spatium1 = staffType1->spatium();
    double lineDistance = staffType1->lineDistance().val() * spatium1;
    double offset = staffType1->yoffset().val() * spatium1;
    double lineWidth = style().styleS(Sid::staffLineWidth).val() * spatium1 * .5;

    double y1 = offset + from * lineDistance * .5 - lineWidth;
    double y2 = offset + (staffType1->lines() * 2 - 2 + to) * lineDistance * .5 + lineWidth;

    if (spanStaff) {
        // we need spatium and line distance of bottom staff
        // as it may be scalled diferently
        const Staff* staff2 = score()->staff(staffIdx2);
        const StaffType* staffType2 = staff2 ? staff2->staffType(tick) : staffType1;
        double spatium2 = staffType2->spatium();
        double lineDistance2 = staffType2->lineDistance().val() * spatium2;
        double startStaffY = system->staff(staffIdx1)->y();

        y2 = measure->staffLines(staffIdx2)->y1() - startStaffY - to * lineDistance2 * 0.5;

        // if bottom staff is single line, set span-to zeropoint to the top of the standard barline
        if (staffType2->lines() <= 1) {
            y2 += BARLINE_SPAN_1LINESTAFF_FROM * lineDistance2 * 0.5;
        }
    }

    // if stafftype change in next measure, check new staff positions
    Fraction tickNext = tick + measure->ticks();
    if (staff1->isStaffTypeStartFrom(tickNext)) {
        Measure* measureNext = measure->nextMeasure();
        System* systemNext = measureNext ? measureNext->system() : nullptr;
        const StaffType* staffType1Next = staff1->staffType(tickNext);
        bool oneLineNext = staffType1Next->lines() <= 1;
        if (systemNext && systemNext == system && staffType1Next != staffType1) {
            if (oneLine && !oneLineNext) {
                from = m_spanFrom;
                to = m_spanTo;
            }
            if (oneLineNext && m_spanFrom == 0 && m_spanTo == 0) {
                from = BARLINE_SPAN_1LINESTAFF_FROM;
                if (!spanStaff) {
                    to = BARLINE_SPAN_1LINESTAFF_TO;
                }
            }
            double spatium1Next = staffType1Next->spatium();
            double lineDistanceNext = staffType1Next->lineDistance().val() * spatium1Next;
            double offsetNext = staffType1Next->yoffset().val() * spatium1Next;
            double lineWidthNext = style().styleS(Sid::staffLineWidth).val() * spatium1Next * .5;

            int y1Next = offsetNext + from * lineDistanceNext * .5 - lineWidthNext;

            // change barline, only if it is not initial one
            if (rtick().isNotZero()) {
                if (y1Next < y1) {
                    y1 = y1Next;
                }

                if (!spanStaff) {
                    double y2Next = offsetNext + (staffType1Next->lines() * 2 - 2 + to) * lineDistanceNext * .5 + lineWidthNext;
                    if (y2Next > y2) {
                        y2 = y2Next;
                    }
                }
            }
        }
    }

    data->y1 = y1;
    data->y2 = y2;
}

//---------------------------------------------------------
//   isTop
//---------------------------------------------------------

bool BarLine::isTop() const
{
    staff_idx_t idx = staffIdx();
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
    if (!m_spanStaff) {
        return true;
    }
    size_t idx = staffIdx();
    if (idx == score()->nstaves() - 1) {
        return true;
    } else {
        return nextVisibleSpannedStaff(this) == idx;
    }
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
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(EditData& data) const
{
    ElementType type = data.dropElement->type();
    if (type == ElementType::BAR_LINE) {
        return true;
    } else {
        return (type == ElementType::FERMATA || type == ElementType::SYMBOL || type == ElementType::IMAGE)
               && segment()
               && segment()->isEndBarLineType();
    }
    // Prevent unreachable code warning
    // return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* BarLine::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;

    if (e->isBarLine()) {
        BarLine* bl    = toBarLine(e);
        BarLineType st = bl->barLineType();

        // check if the new property can apply to this single bar line
        BarLineType bt = BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT;
        bool oldRepeat = barLineType() & bt;
        bool newRepeat = bl->barLineType() & bt;

        Measure* m = measure();
        if (bl->playCount() != -1) {
            m->undoChangeProperty(Pid::REPEAT_COUNT, bl->playCount());
        }

        // if ctrl was used and repeats are not involved,
        // or if drop refers to span rather than subtype =>
        // single bar line drop

        if ((data.control() && !oldRepeat && !newRepeat) || (bl->spanFrom() || bl->spanTo())) {
            // if drop refers to span, update this bar line span
            if (bl->spanFrom() || bl->spanTo()) {
                // if dropped spanFrom or spanTo are below the middle of standard staff (5 lines)
                // adjust to the number of staff lines
                int spanFrom   = bl->spanFrom();
                int spanTo     = bl->spanTo();
                undoChangeProperty(Pid::BARLINE_SPAN, false);
                undoChangeProperty(Pid::BARLINE_SPAN_FROM, spanFrom);
                undoChangeProperty(Pid::BARLINE_SPAN_TO, spanTo);
            }
            // if drop refers to subtype, update this bar line subtype
            else {
                score()->undoChangeBarLineType(this, st, false);
            }
        } else {
            score()->undoChangeBarLineType(this, st, true);
        }
        score()->undoUpdatePlayCountText(m);
        delete e;
    } else if (e->isArticulationFamily()) {
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
        e->setPlacement(track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
        for (EngravingItem* el: segment()->annotations()) {
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

    return style().styleB(Sid::repeatBarTips);
}

void BarLine::setSelected(bool f)
{
    if (f == selected()) {
        return;
    }

    if (playCountText()) {
        playCountText()->setSelected(f);
    }
    EngravingItem::setSelected(f);
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> BarLine::gripsPositions(const EditData& ed) const
{
    const BarLineEditData* bed = static_cast<const BarLineEditData*>(ed.getData(this).get());

    double lw = style().styleMM(Sid::barWidth) * staff()->staffMag(tick());
    const_cast<BarLine*>(this)->calcY();

    const PointF pp = pagePos();

    return {
        //PointF(lw * .5, y1 + bed->yoff1) + pp,
        PointF(lw * .5, ldata()->y2 + bed->yoff2) + pp
    };
}

void BarLine::styleChanged()
{
    if (explicitParent() && m_barLineType == BarLineType::END_REPEAT) {
        score()->undoUpdatePlayCountText(measure());
    }
    EngravingItem::styleChanged();
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void BarLine::startEdit(EditData& ed)
{
    std::shared_ptr<BarLineEditData> bed = std::make_shared<BarLineEditData>();
    bed->e     = this;
    bed->yoff1 = 0;
    bed->yoff2 = 0;
    ed.addData(bed);
}

bool BarLine::isEditAllowed(EditData& ed) const
{
    return (ed.key == Key_Up && spanStaff()) || (ed.key == Key_Down && !spanStaff())
           || EngravingItem::isEditAllowed(ed);
}

bool BarLine::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    bool local = ed.control() || segment()->isBarLineType() || spanStaff() != score()->staff(staffIdx())->barLineSpan();
    if ((ed.key == Key_Up && spanStaff()) || (ed.key == Key_Down && !spanStaff())) {
        if (local) {
            BarLine* b = toBarLine(segment()->element(staffIdx() * VOICES));
            if (b) {
                b->undoChangeProperty(Pid::BARLINE_SPAN, !spanStaff());
            }
        } else {
            score()->staff(staffIdx())->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, !spanStaff());
        }
        return true;
    }
    return EngravingItem::edit(ed);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(EditData& ed)
{
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this).get());

    double lineDist = staff()->lineDistance(tick()) * spatium();
    calcY();
    if (ed.curGrip != Grip::START) {
        return;
    } else {
        // min for bottom grip is 1 line below top grip
        const double min = ldata()->y1 - ldata()->y2 + lineDist;
        // max is the bottom of the system
        const System* system = segment() ? segment()->system() : nullptr;
        const staff_idx_t st = staffIdx();
        const double max = (system && st != muse::nidx)
                           ? (system->height() - ldata()->y2 - system->staff(st)->y())
                           : std::numeric_limits<double>::max();
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
    calcY();
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this).get());
    mutldata()->y1 += bed->yoff1;
    mutldata()->y2 += bed->yoff2;

    double ay0      = pagePos().y();
    double ay2      = ay0 + mutldata()->y2;                       // absolute (page-relative) bar line bottom coord
    staff_idx_t staffIdx1 = staffIdx();
    System* syst   = segment()->measure()->system();
    double systTopY = syst->pagePos().y();

    // determine new span value
    staff_idx_t staffIdx2;
    size_t numOfStaves = syst->staves().size();
    if (staffIdx1 + 1 >= numOfStaves) {
        // if initial staff is last staff, ending staff must be the same
        staffIdx2 = staffIdx1;
    } else {
        // if there are other staves after it, look for staff nearest to bar line bottom coord
        double staff1TopY = syst->staff(staffIdx1)->y() + systTopY;

        for (staffIdx2 = staffIdx1 + 1; staffIdx2 < numOfStaves; ++staffIdx2) {
            // compute 1st staff height, absolute top Y of 2nd staff and height of blank between the staves
            Staff* staff1      = score()->staff(staffIdx2 - 1);
            double staff1Hght    = (staff1->lines(tick()) - 1) * staff1->lineDistance(tick()) * spatium();
            double staff2TopY    = systTopY + syst->staff(staffIdx2)->y();
            double blnkBtwnStaff = staff2TopY - staff1TopY - staff1Hght;
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
    int newSpanFrom = 0;
    int newSpanTo = 0;

    bool localDrag = ed.control() || segment()->isBarLineType() || spanStaff() != score()->staff(staffIdx())->barLineSpan();
    if (localDrag) {
        Segment* s = segment();
        bool breakLast = staffIdx1 == staffIdx2;
        for (staff_idx_t staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
            BarLine* b = toBarLine(s->element(staffIdx * VOICES));
            if (!b) {
                b = toBarLine(linkedClone());
                b->setSpanStaff(true);
                b->setTrack(staffIdx * VOICES);
                b->setParent(s);
                score()->undoAddElement(b);
            }
            breakLast = b->spanTo();
            b->undoChangeProperty(Pid::BARLINE_SPAN, true);
        }
        if (breakLast) {
            BarLine* b = toBarLine(s->element(staffIdx2 * VOICES));
            if (b) {
                b->undoChangeProperty(Pid::BARLINE_SPAN, false);
            }
        }
    } else {
        bool breakLast = staffIdx1 == staffIdx2;
        for (staff_idx_t staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
            breakLast = score()->staff(staffIdx)->barLineSpan();
            score()->staff(staffIdx)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, true);
        }
        if (breakLast) {
            score()->staff(staffIdx2)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
        }
        staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, newSpanFrom);
        staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   newSpanTo);
    }

    bed->yoff1 = 0.0;
    bed->yoff2 = 0.0;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BarLine::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    // if no width (staff has bar lines turned off) and not all requested, do nothing
    if (RealIsNull(width()) && !all) {
        return;
    }

    func(data, this);
    for (EngravingItem* e : m_el) {
        e->scanElements(data, func, all);
    }

    if (m_playCountText) {
        m_playCountText->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void BarLine::setTrack(track_idx_t t)
{
    EngravingItem::setTrack(t);
    for (EngravingItem* e : m_el) {
        e->setTrack(t);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BarLine::add(EngravingItem* e)
{
    e->setParent(this);
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        m_el.push_back(e);
        setGenerated(false);
        e->added();
        break;
    case ElementType::TEXT: {
        Text* t = toText(e);
        t->initElementStyle(&playCountStyle);
        setPlayCountText(toText(e));
    }
    break;
    default:
        LOGD("BarLine::add() not impl. %s", e->typeName());
        delete e;
        break;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BarLine::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        if (!m_el.remove(e)) {
            LOGD("BarLine::remove(): cannot find %s", e->typeName());
        } else {
            e->removed();
        }
        break;
    case ElementType::TEXT:
        setPlayCountText(nullptr);
        break;
    default:
        LOGD("BarLine::remove() not impl. %s", e->typeName());
        return;
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue BarLine::getProperty(Pid id) const
{
    if (EngravingItem* e = const_cast<BarLine*>(this)->propertyDelegate(id)) {
        return e->getProperty(id);
    }

    switch (id) {
    case Pid::BARLINE_TYPE:
        return PropertyValue::fromValue(m_barLineType);
    case Pid::BARLINE_SPAN:
        return spanStaff();
    case Pid::BARLINE_SPAN_FROM:
        return int(spanFrom());
    case Pid::BARLINE_SPAN_TO:
        return int(spanTo());
    case Pid::BARLINE_SHOW_TIPS:
        return showTips();
    case Pid::PLAY_COUNT_TEXT_SETTING:
        return m_playCountTextSetting;
    case Pid::PLAY_COUNT_TEXT:
        return m_playCountCustomText;
    default:
        break;
    }
    return EngravingItem::getProperty(id);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(Pid id, const PropertyValue& v)
{
    if (EngravingItem* e = propertyDelegate(id)) {
        bool success = e->setProperty(id, v);

        if (id == Pid::REPEAT_COUNT) {
            score()->undoUpdatePlayCountText(measure());
        }
        return success;
    }

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
    case Pid::PLAY_COUNT_TEXT_SETTING:
        m_playCountTextSetting = v.value<AutoCustomHide>();
        score()->undoUpdatePlayCountText(measure());
        break;
    case Pid::PLAY_COUNT_TEXT:
        m_playCountCustomText = v.value<String>();
        score()->undoUpdatePlayCountText(measure());
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    setGenerated(false);
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void BarLine::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::BARLINE_TYPE && segment()) {
        score()->undoChangeBarLineType(this, v.value<BarLineType>(), true, true);
    } else {
        EngravingObject::undoChangeProperty(id, v, ps);
    }
}

EngravingItem* BarLine::propertyDelegate(Pid pid)
{
    if (pid == Pid::REPEAT_COUNT) {
        return measure();
    }

    return EngravingItem::propertyDelegate(pid);
}

void BarLine::setPlayCountText(Text* text)
{
    m_playCountText = text;
    if (m_playCountText != nullptr) {
        setGenerated(false);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue BarLine::propertyDefault(Pid propertyId) const
{
    if (EngravingItem* e = const_cast<BarLine*>(this)->propertyDelegate(propertyId)) {
        return e->propertyDefault(propertyId);
    }

    switch (propertyId) {
    case Pid::BARLINE_TYPE:
// dynamic default values are a bad idea: writing to xml the value maybe omitted resulting in
//    wrong values on read (as the default may be different on read)
//                  if (segment() && segment()->measure() && !segment()->measure()->nextMeasure())
//                        return QVariant::fromValue(BarLineType::END);
        return PropertyValue::fromValue(BarLineType::NORMAL);

    case Pid::BARLINE_SPAN:
        return staff() ? staff()->barLineSpan() : false;

    case Pid::BARLINE_SPAN_FROM:
        return staff() ? staff()->barLineFrom() : 0;

    case Pid::BARLINE_SPAN_TO:
        return staff() ? staff()->barLineTo() : 0;

    case Pid::BARLINE_SHOW_TIPS:
        return false;

    case Pid::PLAY_COUNT_TEXT_SETTING:
        return AutoCustomHide::AUTO;

    case Pid::PLAY_COUNT_TEXT:
        return String();

    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* BarLine::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());      //score()->inputState().prevTrack() / VOICES);
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* BarLine::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());       //score()->inputState().prevTrack() / VOICES);
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString BarLine::subtypeUserName() const
{
    for (const auto& i : barLineTable) {
        if (i.type == barLineType()) {
            return i.userName;
        }
    }
    return TranslatableString();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String BarLine::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), BarLine::translatedUserTypeName(barLineType()));
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String BarLine::accessibleExtraInfo() const
{
    Segment* seg = segment();
    String rez;

    for (const EngravingItem* e : *el()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
    }

    for (const EngravingItem* e : seg->annotations()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        if (e->track() == track()) {
            rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
        }
    }
    Measure* m = seg->measure();

    if (m) {      // always true?
        //jumps
        for (const EngravingItem* e : m->el()) {
            if (!score()->selectionFilter().canSelect(e)) {
                continue;
            }
            if (e->type() == ElementType::JUMP) {
                rez= String(u"%1 %2").arg(rez, e->screenReaderInfo());
            }
            if (e->type() == ElementType::MARKER) {
                const Marker* m1 = toMarker(e);
                if (m1->markerType() == MarkerType::FINE) {
                    rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
                }
            }
        }
        //markers
        Measure* nextM = m->nextMeasureMM();
        if (nextM) {
            for (const EngravingItem* e : nextM->el()) {
                if (!score()->selectionFilter().canSelect(e)) {
                    continue;
                }
                if (e->isMarker()) {
                    if (toMarker(e)->markerType() == MarkerType::FINE) {
                        continue;             //added above^
                    }
                    rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
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
                rez += u"; " + muse::mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
            }
            if (s->tick2() == tick) {
                rez += u"; " + muse::mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
            }
        }
    }
    return rez;
}
