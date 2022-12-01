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

/**
 \file
 Implementation of classes SysStaff and System.
*/

#include "system.h"

#include "style/style.h"
#include "style/defaultstyle.h"
#include "rw/xml.h"
#include "layout/layoutcontext.h"
#include "realfn.h"

#include "barline.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "bracketItem.h"
#include "chord.h"
#include "chordrest.h"
#include "factory.h"
#include "instrumentname.h"
#include "measure.h"
#include "mscore.h"
#include "page.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "spacer.h"
#include "spanner.h"
#include "staff.h"
#include "system.h"
#include "systemdivider.h"
#include "textframe.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
{
    DeleteAll(instrumentNames);
}

//---------------------------------------------------------
//   yBottom
//---------------------------------------------------------

double SysStaff::yBottom() const
{
    return skyline().south().valid() ? skyline().south().max() : _height;
}

//---------------------------------------------------------
//   saveLayout
//---------------------------------------------------------

void SysStaff::saveLayout()
{
    _height =  bbox().height();
    _yPos = bbox().y();
}

//---------------------------------------------------------
//   saveLayout
//---------------------------------------------------------

void SysStaff::restoreLayout()
{
    bbox().setTop(_yPos);
    bbox().setHeight(_height);
}

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Page* parent)
    : EngravingItem(ElementType::SYSTEM, parent)
{
}

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
{
    for (SpannerSegment* ss : spannerSegments()) {
        if (ss->system() == this) {
            ss->resetExplicitParent();
        }
    }
    for (MeasureBase* mb : measures()) {
        if (mb->system() == this) {
            mb->resetExplicitParent();
        }
    }
    DeleteAll(_staves);
    DeleteAll(_brackets);
    delete _systemDividerLeft;
    delete _systemDividerRight;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr System::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

void System::moveToPage(Page* parent)
{
    setParent(parent);
}

//---------------------------------------------------------
///   clear
///   Clear layout of System
//---------------------------------------------------------

void System::clear()
{
    for (MeasureBase* mb : measures()) {
        if (mb->system() == this) {
            mb->resetExplicitParent();
        }
    }
    ml.clear();
    for (SpannerSegment* ss : _spannerSegments) {
        if (ss->system() == this) {
            ss->resetExplicitParent();             // assume parent() is System
        }
    }
    _spannerSegments.clear();
    // _systemDividers are reused
}

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

void System::appendMeasure(MeasureBase* mb)
{
    assert(!mb->isMeasure() || !(score()->styleB(Sid::createMultiMeasureRests) && toMeasure(mb)->hasMMRest()));
    mb->setParent(this);
    ml.push_back(mb);
}

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void System::removeMeasure(MeasureBase* mb)
{
    ml.erase(std::remove(ml.begin(), ml.end(), mb), ml.end());
    if (mb->system() == this) {
        mb->resetExplicitParent();
    }
}

//---------------------------------------------------------
//   removeLastMeasure
//---------------------------------------------------------

void System::removeLastMeasure()
{
    if (ml.empty()) {
        return;
    }
    MeasureBase* mb = ml.back();
    ml.pop_back();
    if (mb->system() == this) {
        mb->resetExplicitParent();
    }
}

//---------------------------------------------------------
//   vbox
//    a system can only contain one vertical frame
//---------------------------------------------------------

Box* System::vbox() const
{
    if (!ml.empty()) {
        if (ml[0]->isVBox() || ml[0]->isTBox()) {
            return toBox(ml[0]);
        }
    }
    return 0;
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

SysStaff* System::insertStaff(int idx)
{
    SysStaff* staff = new SysStaff;
    if (idx) {
        // HACK: guess position
        staff->bbox().setTop(_staves[idx - 1]->y() + 6 * spatium());
    }
    _staves.insert(_staves.begin() + idx, staff);
    return staff;
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void System::removeStaff(int idx)
{
    _staves.erase(_staves.begin() + idx);
}

//---------------------------------------------------------
//   adjustStavesNumber
//---------------------------------------------------------

void System::adjustStavesNumber(size_t nstaves)
{
    for (size_t i = _staves.size(); i < nstaves; ++i) {
        insertStaff(static_cast<int>(i));
    }
    const size_t dn = _staves.size() - nstaves;
    for (size_t i = 0; i < dn; ++i) {
        removeStaff(static_cast<int>(_staves.size()) - 1);
    }
}

//---------------------------------------------------------
//   instrumentNamesWidth
//---------------------------------------------------------

double System::instrumentNamesWidth()
{
    double namesWidth = 0.0;

    for (staff_idx_t staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
        const SysStaff* staff = this->staff(staffIdx);
        if (!staff) {
            continue;
        }

        for (InstrumentName* name : staff->instrumentNames) {
            name->layout();
            namesWidth = std::max(namesWidth, name->width());
        }
    }

    return namesWidth;
}

//---------------------------------------------------------
//   layoutBrackets
//---------------------------------------------------------

double System::layoutBrackets(const LayoutContext& ctx)
{
    size_t nstaves  = _staves.size();
    size_t columns = getBracketsColumnsCount();

#if (!defined (_MSCVER) && !defined (_MSC_VER))
    double bracketWidth[columns];
#else
    // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
    //    heap allocation is slow, an optimization might be used.
    std::vector<double> bracketWidth(columns);
#endif
    for (size_t i = 0; i < columns; ++i) {
        bracketWidth[i] = 0.0;
    }

    std::vector<Bracket*> bl;
    bl.swap(_brackets);

    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        Staff* s = score()->staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                Bracket* b = createBracket(ctx, bi, i, static_cast<int>(staffIdx), bl, this->firstMeasure());
                if (b != nullptr) {
                    bracketWidth[i] = std::max(bracketWidth[i], b->width());
                }
            }
        }
    }

    for (Bracket* b : bl) {
        delete b;
    }

    double totalBracketWidth = 0.0;

    if (!_brackets.empty()) {
        for (double w : bracketWidth) {
            totalBracketWidth += w;
        }
    }

    return totalBracketWidth;
}

//---------------------------------------------------------
//   totalBracketOffset
//---------------------------------------------------------

/// Calculates the total width of all brackets together that
/// would be visible when all staves are visible.
/// The logic in this method is closely related to the logic in
/// System::layoutBrackets and System::createBracket.
double System::totalBracketOffset(LayoutContext& ctx)
{
    if (ctx.totalBracketsWidth >= 0) {
        return ctx.totalBracketsWidth;
    }

    size_t columns = 0;
    for (const Staff* staff : ctx.score()->staves()) {
        for (auto bi : staff->brackets()) {
            columns = std::max(columns, bi->column() + 1);
        }
    }

    size_t nstaves = ctx.score()->nstaves();
    std::vector < double > bracketWidth(nstaves, 0.0);
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* staff = ctx.score()->staff(staffIdx);
        for (auto bi : staff->brackets()) {
            if (bi->bracketType() == BracketType::NO_BRACKET) {
                continue;
            }

            //! This logic is partially copied from System::createBracket.
            //! Of course, we don't need to worry about invisible staves,
            //! but we do need to worry about brackets that span past the
            //! last staff.
            staff_idx_t firstStaff = staffIdx;
            staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
            if (lastStaff >= nstaves) {
                lastStaff = nstaves - 1;
            }

            for (; firstStaff <= lastStaff; ++firstStaff) {
                if (ctx.score()->staff(firstStaff)->show()) {
                    break;
                }
            }
            for (; lastStaff >= firstStaff; --lastStaff) {
                if (ctx.score()->staff(lastStaff)->show()) {
                    break;
                }
            }

            size_t span = lastStaff - firstStaff + 1;
            if (span > 1
                || (bi->bracketSpan() == span)
                || (span == 1 && ctx.score()->styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden))) {
                Bracket* dummyBr = Factory::createBracket(ctx.score()->dummy(), /*isAccessibleEnabled=*/ false);
                dummyBr->setBracketItem(bi);
                dummyBr->setStaffSpan(firstStaff, lastStaff);
                dummyBr->layout();
                bracketWidth[staffIdx] += dummyBr->width();
                delete dummyBr;
            }
        }
    }

    double totalBracketsWidth = 0.0;
    for (double w : bracketWidth) {
        totalBracketsWidth = std::max(totalBracketsWidth, w);
    }
    ctx.totalBracketsWidth = totalBracketsWidth;

    return ctx.totalBracketsWidth;
}

//---------------------------------------------------------
//   layoutSystem
///   Layout the System
//---------------------------------------------------------

void System::layoutSystem(LayoutContext& ctx, double xo1, const bool isFirstSystem, bool firstSystemIndent)
{
    if (_staves.empty()) {                 // ignore vbox
        return;
    }

    // Get standard instrument name distance
    double instrumentNameOffset = score()->styleMM(Sid::instrumentNameOffset);
    // Now scale it depending on the text size (which also may not follow staff scaling)
    double textSizeScaling = 1.0;
    double actualSize = 0.0;
    double defaultSize = 0.0;
    bool followStaffSize = true;
    if (ctx.startWithLongNames) {
        actualSize = score()->styleD(Sid::longInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::longInstrumentFontSize).toDouble();
        followStaffSize = score()->styleB(Sid::longInstrumentFontSpatiumDependent);
    } else {
        actualSize = score()->styleD(Sid::shortInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::shortInstrumentFontSize).toDouble();
        followStaffSize = score()->styleB(Sid::shortInstrumentFontSpatiumDependent);
    }
    textSizeScaling = actualSize / defaultSize;
    if (!followStaffSize) {
        textSizeScaling *= DefaultStyle::defaultStyle().value(Sid::spatium).toDouble() / score()->styleD(Sid::spatium);
    }
    textSizeScaling = std::max(textSizeScaling, 1.0);
    instrumentNameOffset *= textSizeScaling;

    size_t nstaves  = _staves.size();

    //---------------------------------------------------
    //  find x position of staves
    //---------------------------------------------------
    layoutBrackets(ctx);
    double maxBracketsWidth = totalBracketOffset(ctx);

    double maxNamesWidth = instrumentNamesWidth();

    double indent = maxNamesWidth > 0 ? maxNamesWidth + instrumentNameOffset : 0.0;
    if (isFirstSystem && firstSystemIndent) {
        indent = std::max(indent, styleP(Sid::firstSystemIndentationValue) * mag() - maxBracketsWidth);
        maxNamesWidth = indent - instrumentNameOffset;
    }

    if (RealIsNull(indent)) {
        if (score()->styleB(Sid::alignSystemToMargin)) {
            _leftMargin = 0.0;
        } else {
            _leftMargin = maxBracketsWidth;
        }
    } else {
        _leftMargin = indent + maxBracketsWidth;
    }

    int nVisible = 0;
    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        SysStaff* s  = _staves[staffIdx];
        Staff* staff = score()->staff(staffIdx);
        if (!staff->show() || !s->show()) {
            s->setbbox(RectF());
            continue;
        }
        ++nVisible;
        double staffMag = staff->staffMag(Fraction(0, 1));         // ??? TODO
        int staffLines = staff->lines(Fraction(0, 1));
        if (staffLines <= 1) {
            double h = staff->lineDistance(Fraction(0, 1)) * staffMag * spatium();
            s->bbox().setRect(_leftMargin + xo1, -h, 0.0, 2 * h);
        } else {
            double h = (staffLines - 1) * staff->lineDistance(Fraction(0, 1));
            h = h * staffMag * spatium();
            s->bbox().setRect(_leftMargin + xo1, 0.0, 0.0, h);
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------

    setBracketsXPosition(xo1 + _leftMargin);

    //---------------------------------------------------
    //  layout instrument names x position
    //     at this point it is not clear which staves will
    //     be hidden, so layout all instrument names
    //---------------------------------------------------

    for (SysStaff* s : _staves) {
        for (InstrumentName* t : s->instrumentNames) {
            t->layout();

            switch (t->align().horizontal) {
            case AlignH::LEFT:
                t->setPosX(0);
                break;
            case AlignH::HCENTER:
                t->setPosX(maxNamesWidth * .5);
                break;
            case AlignH::RIGHT:
                t->setPosX(maxNamesWidth);
                break;
            }
        }
    }

    for (MeasureBase* mb : measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        if (m == measures().front() || (m->prev() && m->prev()->isHBox())) {
            m->createSystemBeginBarLine();
        }
    }
}

//---------------------------------------------------------
//   setMeasureHeight
//---------------------------------------------------------

void System::setMeasureHeight(double height)
{
    double _spatium { spatium() };
    for (MeasureBase* m : ml) {
        if (m->isMeasure()) {
            // note that the factor 2 * _spatium must be corrected for when exporting
            // system distance in MusicXML (issue #24733)
            m->bbox().setRect(0.0, -_spatium, m->width(), height + 2.0 * _spatium);
        } else if (m->isHBox()) {
            m->bbox().setRect(0.0, 0.0, m->width(), height);
            toHBox(m)->layout2();
        } else if (m->isTBox()) {
            toTBox(m)->layout();
        } else {
            LOGD("unhandled measure type %s", m->typeName());
        }
    }
}

//---------------------------------------------------------
//   layoutBracketsVertical
//---------------------------------------------------------

void System::layoutBracketsVertical()
{
    for (Bracket* b : _brackets) {
        int staffIdx1 = static_cast<int>(b->firstStaff());
        int staffIdx2 = static_cast<int>(b->lastStaff());
        double sy = 0;                           // assume bracket not visible
        double ey = 0;
        // if start staff not visible, try next staff
        while (staffIdx1 <= staffIdx2 && !_staves[staffIdx1]->show()) {
            ++staffIdx1;
        }
        // if end staff not visible, try prev staff
        while (staffIdx1 <= staffIdx2 && !_staves[staffIdx2]->show()) {
            --staffIdx2;
        }
        // if the score doesn't have "alwaysShowBracketsWhenEmptyStavesAreHidden" as true,
        // the bracket will be shown IF:
        // it spans at least 2 visible staves (staffIdx1 < staffIdx2) OR
        // it spans just one visible staff (staffIdx1 == staffIdx2) but it is required to do so
        // (the second case happens at least when the bracket is initially dropped)
        bool notHidden = score()->styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
                         ? (staffIdx1 <= staffIdx2) : (staffIdx1 < staffIdx2) || (b->span() == 1 && staffIdx1 == staffIdx2);
        if (notHidden) {                        // set vert. pos. and height to visible spanned staves
            sy = _staves[staffIdx1]->bbox().top();
            ey = _staves[staffIdx2]->bbox().bottom();
        }
        b->setPosY(sy);
        b->setHeight(ey - sy);
        b->layout();
    }
}

//---------------------------------------------------------
//   layoutInstrumentNames
//---------------------------------------------------------

void System::layoutInstrumentNames()
{
    staff_idx_t staffIdx = 0;

    for (Part* p : score()->parts()) {
        SysStaff* s = staff(staffIdx);
        SysStaff* s2;
        size_t nstaves = p->nstaves();

        staff_idx_t visible = firstVisibleSysStaffOfPart(p);
        if (visible != mu::nidx) {
            // The top staff might be invisible but this top staff contains the instrument names.
            // To make sure these instrument name are drawn, even when the top staff is invisible,
            // move the InstrumentName elements to the first visible staff of the part.
            if (visible != staffIdx) {
                SysStaff* vs = staff(visible);
                for (InstrumentName* t : s->instrumentNames) {
                    t->setTrack(visible * VOICES);
                    t->setSysStaff(vs);
                    vs->instrumentNames.push_back(t);
                }
                s->instrumentNames.clear();
                s = vs;
            }

            for (InstrumentName* t : s->instrumentNames) {
                //
                // override Text->layout()
                //
                double y1, y2;
                switch (t->layoutPos()) {
                default:
                case 0:                         // center at part
                    y1 = s->bbox().top();
                    s2 = staff(staffIdx);
                    for (int i = static_cast<int>(staffIdx + nstaves - 1); i > 0; --i) {
                        SysStaff* s3 = staff(i);
                        if (s3->show()) {
                            s2 = s3;
                            break;
                        }
                    }
                    y2 = s2->bbox().bottom();
                    break;
                case 1:                         // center at first staff
                    y1 = s->bbox().top();
                    y2 = s->bbox().bottom();
                    break;
                case 2:                         // center between first and second staff
                    y1 = s->bbox().top();
                    y2 = staff(staffIdx + 1)->bbox().bottom();
                    break;
                case 3:                         // center at second staff
                    y1 = staff(staffIdx + 1)->bbox().top();
                    y2 = staff(staffIdx + 1)->bbox().bottom();
                    break;
                case 4:                         // center between first and second staff
                    y1 = staff(staffIdx + 1)->bbox().top();
                    y2 = staff(staffIdx + 2)->bbox().bottom();
                    break;
                case 5:                         // center at third staff
                    y1 = staff(staffIdx + 2)->bbox().top();
                    y2 = staff(staffIdx + 2)->bbox().bottom();
                    break;
                }
                t->setPosY(y1 + (y2 - y1) * .5 + t->offset().y());
            }
        }
        staffIdx += nstaves;
    }
}

//---------------------------------------------------------
//   addBrackets
//   Add brackets in front of this measure, typically behind a HBox
//---------------------------------------------------------

void System::addBrackets(const LayoutContext& ctx, Measure* measure)
{
    if (_staves.empty()) {                 // ignore vbox
        return;
    }

    size_t nstaves = _staves.size();

    //---------------------------------------------------
    //  find x position of staves
    //    create brackets
    //---------------------------------------------------

    size_t columns = getBracketsColumnsCount();

    std::vector<Bracket*> bl;
    bl.swap(_brackets);

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        Staff* s = score()->staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                createBracket(ctx, bi, i, staffIdx, bl, measure);
            }
        }
        if (!staff(staffIdx)->show()) {
            continue;
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------

    setBracketsXPosition(measure->x());

    mu::join(_brackets, bl);
}

//---------------------------------------------------------
//   createBracket
//   Create a bracket if it spans more then one visible system
//   If measure is NULL adds the bracket in front of the system, else in front of the measure.
//   Returns the bracket if it got created, else NULL
//---------------------------------------------------------

Bracket* System::createBracket(const LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                               std::vector<Bracket*>& bl,
                               Measure* measure)
{
    size_t nstaves = _staves.size();
    staff_idx_t firstStaff = staffIdx;
    staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
    if (lastStaff >= nstaves) {
        lastStaff = nstaves - 1;
    }

    for (; firstStaff <= lastStaff; ++firstStaff) {
        if (staff(firstStaff)->show()) {
            break;
        }
    }
    for (; lastStaff >= firstStaff; --lastStaff) {
        if (staff(lastStaff)->show()) {
            break;
        }
    }
    size_t span = lastStaff - firstStaff + 1;
    //
    // do not show bracket if it only spans one
    // system due to some invisible staves
    //
    if (span > 1
        || (bi->bracketSpan() == span)
        || (span == 1 && score()->styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)) && bi->bracketType() != BracketType::SQUARE
        || (span == 1 && score()->styleB(Sid::alwaysShowSquareBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() == BracketType::SQUARE)) {
        //
        // this bracket is visible
        //
        Bracket* b = 0;
        track_idx_t track = staffIdx * VOICES;
        for (size_t k = 0; k < bl.size(); ++k) {
            if (bl[k]->track() == track && bl[k]->column() == column && bl[k]->bracketType() == bi->bracketType()
                && bl[k]->measure() == measure) {
                b = mu::takeAt(bl, k);
                break;
            }
        }
        if (b == 0) {
            b = Factory::createBracket(ctx.score()->dummy());
            b->setBracketItem(bi);
            b->setGenerated(true);
            b->setTrack(track);
            b->setMeasure(measure);
        }
        add(b);

        if (bi->selected()) {
            bool needSelect = true;

            std::vector<EngravingItem*> brackets = score()->selection().elements(ElementType::BRACKET);
            for (const EngravingItem* element : brackets) {
                if (toBracket(element)->bracketItem() == bi) {
                    needSelect = false;
                    break;
                }
            }

            if (needSelect) {
                score()->select(b, SelectType::ADD);
            }
        }

        b->setStaffSpan(firstStaff, lastStaff);

        return b;
    }

    return nullptr;
}

size_t System::getBracketsColumnsCount()
{
    size_t columns = 0;
    for (const Staff* staff : score()->staves()) {
        for (auto bi : staff->brackets()) {
            columns = std::max(columns, bi->column() + 1);
        }
    }
    return columns;
}

void System::setBracketsXPosition(const double xPosition)
{
    for (Bracket* b1 : _brackets) {
        // Obtain correct distance
        double bracketDistance = 0.0;
        BracketType bracketType = b1->bracketType();
        if (bracketType == BracketType::NORMAL || bracketType == BracketType::LINE) {
            bracketDistance = score()->styleMM(Sid::bracketDistance);
        } else if (bracketType == BracketType::BRACE) {
            bracketDistance = score()->styleMM(Sid::akkoladeBarDistance);
        }
        // For brackets that are drawn, we must correct for half line width
        double lineWidthCorrection = 0.0;
        if (bracketType == BracketType::NORMAL || bracketType == BracketType::LINE) {
            lineWidthCorrection = score()->styleMM(Sid::bracketWidth) / 2;
        }
        // Compute offset cause by other stacked brackets
        double xOffset = 0;
        for (const Bracket* b2 : _brackets) {
            bool b1FirstStaffInB2 = (b1->firstStaff() >= b2->firstStaff() && b1->firstStaff() <= b2->lastStaff());
            bool b1LastStaffInB2 = (b1->lastStaff() >= b2->firstStaff() && b1->lastStaff() <= b2->lastStaff());
            if (b1->column() > b2->column()
                && (b1FirstStaffInB2 || b1LastStaffInB2)) {
                xOffset += b2->width();
            }
        }
        // Set position
        b1->setPosX(xPosition - xOffset - b1->width() + lineWidthCorrection);
    }
}

//---------------------------------------------------------
//   nextVisibleStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleStaffFrom(staff_idx_t startStaffIdx) const
{
    for (staff_idx_t i = startStaffIdx; i < _staves.size(); ++i) {
        Staff* s  = score()->staff(i);
        SysStaff* ss = _staves[i];

        if (s->show() && ss->show()) {
            return i;
        }
    }

    return mu::nidx;
}

staff_idx_t System::nextVisibleStaff(staff_idx_t staffIdx) const
{
    return firstVisibleStaffFrom(staffIdx + 1);
}

//---------------------------------------------------------
//   firstVisibleStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleStaff() const
{
    return firstVisibleStaffFrom(0);
}

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void System::layout2(const LayoutContext& ctx)
{
    Box* vb = vbox();
    if (vb) {
        vb->layout();
        setbbox(vb->bbox());
        return;
    }

    setPos(0.0, 0.0);
    std::list<std::pair<size_t, SysStaff*> > visibleStaves;

    for (size_t i = 0; i < _staves.size(); ++i) {
        Staff* s  = score()->staff(i);
        SysStaff* ss = _staves[i];
        if (s->show() && ss->show()) {
            visibleStaves.push_back(std::pair<size_t, SysStaff*>(i, ss));
        } else {
            ss->setbbox(RectF());        // already done in layout() ?
        }
    }

    double _spatium            = spatium();
    double y                   = 0.0;
    double minVerticalDistance = score()->styleMM(Sid::minVerticalDistance);
    double staffDistance       = score()->styleMM(Sid::staffDistance);
    double akkoladeDistance    = score()->styleMM(Sid::akkoladeDistance);
    if (score()->enableVerticalSpread()) {
        staffDistance       = score()->styleMM(Sid::minStaffSpread);
        akkoladeDistance    = score()->styleMM(Sid::minStaffSpread);
    }

    if (visibleStaves.empty()) {
        return;
    }

    for (auto i = visibleStaves.begin();; ++i) {
        SysStaff* ss  = i->second;
        staff_idx_t si1 = i->first;
        Staff* staff  = score()->staff(si1);
        auto ni       = std::next(i);

        double dist = staff->height();
        double yOffset;
        double h;
        if (staff->lines(Fraction(0, 1)) == 1) {
            yOffset = _spatium * BARLINE_SPAN_1LINESTAFF_TO * 0.5;
            h = _spatium * (BARLINE_SPAN_1LINESTAFF_TO - BARLINE_SPAN_1LINESTAFF_FROM) * 0.5;
        } else {
            yOffset = 0.0;
            h = staff->height();
        }
        if (ni == visibleStaves.end()) {
            ss->setYOff(yOffset);
            ss->bbox().setRect(_leftMargin, y - yOffset, width() - _leftMargin, h);
            ss->saveLayout();
            break;
        }

        staff_idx_t si2 = ni->first;
        Staff* staff2  = score()->staff(si2);

        if (staff->part() == staff2->part()) {
            Measure* m = firstMeasure();
            double mag = m ? staff->staffMag(m->tick()) : 1.0;
            dist += akkoladeDistance * mag;
        } else {
            dist += staffDistance;
        }
        dist += staff2->userDist();
        bool fixedSpace = false;
        for (MeasureBase* mb : ml) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            Spacer* sp = m->vspacerDown(si1);
            if (sp) {
                if (sp->spacerType() == SpacerType::FIXED) {
                    dist = staff->height() + sp->gap();
                    fixedSpace = true;
                    break;
                } else {
                    dist = std::max(dist, staff->height() + sp->gap());
                }
            }
            sp = m->vspacerUp(si2);
            if (sp) {
                dist = std::max(dist, sp->gap() + staff->height());
            }
        }
        if (!fixedSpace) {
            // check minimum distance to next staff
            // note that in continuous view, we normally only have a partial skyline for the system
            // a full one is only built when triggering a full layout
            // therefore, we don't know the value we get from minDistance will actually be enough
            // so we remember the value between layouts and increase it when necessary
            // (the first layout on switching to continuous view gives us good initial values)
            // the result is space is good to start and grows as needed
            // it does not, however, shrink when possible - only by trigger a full layout
            // (such as by toggling to page view and back)
            double d = ss->skyline().minDistance(System::staff(si2)->skyline());
            if (score()->lineMode()) {
                double previousDist = ss->continuousDist();
                if (d > previousDist) {
                    ss->setContinuousDist(d);
                } else {
                    d = previousDist;
                }
            }
            dist = std::max(dist, d + minVerticalDistance);
        }
        ss->setYOff(yOffset);
        ss->bbox().setRect(_leftMargin, y - yOffset, width() - _leftMargin, h);
        ss->saveLayout();
        y += dist;
    }

    _systemHeight = staff(visibleStaves.back().first)->bbox().bottom();
    setHeight(_systemHeight);

    setMeasureHeight(_systemHeight);

    //---------------------------------------------------
    //  layout brackets vertical position
    //---------------------------------------------------

    layoutBracketsVertical();

    //---------------------------------------------------
    //  layout instrument names
    //---------------------------------------------------

    layoutInstrumentNames();

    //---------------------------------------------------
    //  layout cross-staff slurs and ties
    //---------------------------------------------------

    Fraction stick = measures().front()->tick();
    Fraction etick = measures().back()->endTick();
    auto spanners = ctx.score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());

    std::vector<Spanner*> spanner;
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->tick() < etick && sp->tick2() >= stick) {
            if (sp->isSlur()) {
                ChordRest* scr = sp->startCR();
                ChordRest* ecr = sp->endCR();
                staff_idx_t idx = sp->vStaffIdx();
                if (scr && ecr && (scr->vStaffIdx() != idx || ecr->vStaffIdx() != idx)) {
                    sp->layoutSystem(this);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   restoreLayout2
//---------------------------------------------------------

void System::restoreLayout2()
{
    if (vbox()) {
        return;
    }

    for (SysStaff* s : _staves) {
        s->restoreLayout();
    }

    setHeight(_systemHeight);
    setMeasureHeight(_systemHeight);
}

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void System::setInstrumentNames(const LayoutContext& ctx, bool longName, Fraction tick)
{
    //
    // remark: add/remove instrument names is not undo/redoable
    //         as add/remove of systems is not undoable
    //
    if (vbox()) {                 // ignore vbox
        return;
    }
    if (!score()->showInstrumentNames()
        || (style()->styleB(Sid::hideInstrumentNameIfOneInstrument) && score()->visiblePartCount() <= 1)) {
        for (SysStaff* staff : _staves) {
            for (InstrumentName* t : staff->instrumentNames) {
                ctx.score()->removeElement(t);
            }
        }
        return;
    }

    int staffIdx = 0;
    for (SysStaff* staff : _staves) {
        Staff* s = score()->staff(staffIdx);
        if (!s->isTop() || !s->show()) {
            for (InstrumentName* t : staff->instrumentNames) {
                ctx.score()->removeElement(t);
            }
            ++staffIdx;
            continue;
        }

        Part* part = s->part();
        const std::list<StaffName>& names = longName ? part->longNames(tick) : part->shortNames(tick);

        size_t idx = 0;
        for (const StaffName& sn : names) {
            InstrumentName* iname = mu::value(staff->instrumentNames, idx);
            if (iname == 0) {
                iname = new InstrumentName(this);
                // iname->setGenerated(true);
                iname->setParent(this);
                iname->setSysStaff(staff);
                iname->setTrack(staffIdx * VOICES);
                iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
                iname->setLayoutPos(sn.pos());
                ctx.score()->addElement(iname);
            }
            iname->setXmlText(sn.name());
            ++idx;
        }
        for (; idx < staff->instrumentNames.size(); ++idx) {
            ctx.score()->removeElement(staff->instrumentNames[idx]);
        }
        ++staffIdx;
    }
}

//---------------------------------------------------------
//   y2staff
//---------------------------------------------------------

/**
 Return staff number for canvas relative y position \a y
 or -1 if not found.

 To allow drag and drop above and below the staff, the actual y range
 considered "inside" the staff is increased by "margin".
*/

int System::y2staff(double y) const
{
    y -= pos().y();
    int idx = 0;
    double margin = spatium() * 2;
    for (SysStaff* s : _staves) {
        double y1 = s->bbox().top() - margin;
        double y2 = s->bbox().bottom() + margin;
        if (y >= y1 && y < y2) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

//---------------------------------------------------------
//   searchStaff
///   Finds a staff which y position is most close to the
///   given \p y.
///   \param y The y coordinate in system coordinates.
///   \param preferredStaff If not -1, will give more space
///   to a staff with the given number when searching it by
///   coordinate.
///   \returns Number of the found staff.
//---------------------------------------------------------

staff_idx_t System::searchStaff(double y, staff_idx_t preferredStaff /* = invalid */, double spacingFactor) const
{
    staff_idx_t i = 0;
    const size_t nstaves = score()->nstaves();
    for (; i < nstaves;) {
        SysStaff* stff = staff(i);
        if (!stff->show() || !score()->staff(i)->show()) {
            ++i;
            continue;
        }
        staff_idx_t ni = i;
        for (;;) {
            ++ni;
            if (ni == nstaves || (staff(ni)->show() && score()->staff(ni)->show())) {
                break;
            }
        }

        double sy2;
        if (ni != nstaves) {
            SysStaff* nstaff = staff(ni);
            double s1y2       = stff->bbox().y() + stff->bbox().height();
            if (i == preferredStaff) {
                sy2 = s1y2 + (nstaff->bbox().y() - s1y2);
            } else if (ni == preferredStaff) {
                sy2 = s1y2;
            } else {
                sy2 = s1y2 + (nstaff->bbox().y() - s1y2) * spacingFactor;
            }
        } else {
            sy2 = page()->height() - pos().y();
        }
        if (y > sy2) {
            i   = ni;
            continue;
        }
        break;
    }
    return i;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void System::add(EngravingItem* el)
{
    if (!el) {
        return;
    }
// LOGD("%p System::add: %p %s", this, el, el->typeName());

    el->setParent(this);

    switch (el->type()) {
    case ElementType::INSTRUMENT_NAME:
// LOGD("  staffIdx %d, staves %d", el->staffIdx(), _staves.size());
        _staves[el->staffIdx()]->instrumentNames.push_back(toInstrumentName(el));
        toInstrumentName(el)->setSysStaff(_staves[el->staffIdx()]);
        break;

    case ElementType::BEAM:
        score()->addElement(el);
        break;

    case ElementType::BRACKET: {
        Bracket* b   = toBracket(el);
        _brackets.push_back(b);
    }
    break;

    case ElementType::MEASURE:
    case ElementType::HBOX:
    case ElementType::VBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
        score()->addElement(el);
        break;
    case ElementType::TEXTLINE_SEGMENT:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::OTTAVA_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::VOLTA_SEGMENT:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
    case ElementType::WHAMMY_BAR_SEGMENT:
    case ElementType::RASGUEADO_SEGMENT:
    case ElementType::HARMONIC_MARK_SEGMENT:
    case ElementType::PICK_SCRAPE_SEGMENT:
    {
        SpannerSegment* ss = toSpannerSegment(el);
#ifndef NDEBUG
        if (mu::contains(_spannerSegments, ss)) {
            LOGD("System::add() %s %p already there", ss->typeName(), ss);
        } else
#endif
        _spannerSegments.push_back(ss);
    }
    break;

    case ElementType::SYSTEM_DIVIDER:
    {
        SystemDivider* sd = toSystemDivider(el);
        if (sd->dividerType() == SystemDivider::Type::LEFT) {
            _systemDividerLeft = sd;
        } else {
            _systemDividerRight = sd;
        }
    }
    break;

    default:
        LOGD("System::add(%s) not implemented", el->typeName());
        return;
    }

    el->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(EngravingItem* el)
{
    switch (el->type()) {
    case ElementType::INSTRUMENT_NAME:
        mu::remove(_staves[el->staffIdx()]->instrumentNames, toInstrumentName(el));
        toInstrumentName(el)->setSysStaff(0);
        break;
    case ElementType::BEAM:
        score()->removeElement(el);
        break;
    case ElementType::BRACKET:
    {
        Bracket* b = toBracket(el);
        if (!mu::remove(_brackets, b)) {
            LOGD("System::remove: bracket not found");
        }
    }
    break;
    case ElementType::MEASURE:
    case ElementType::HBOX:
    case ElementType::VBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
        score()->removeElement(el);
        break;
    case ElementType::TEXTLINE_SEGMENT:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::OTTAVA_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::VOLTA_SEGMENT:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
        if (!mu::remove(_spannerSegments, toSpannerSegment(el))) {
            LOGD("System::remove: %p(%s) not found, score %p", el, el->typeName(), score());
            assert(score() == el->score());
        }
        break;
    case ElementType::SYSTEM_DIVIDER:
        if (el == _systemDividerLeft) {
            _systemDividerLeft = 0;
        } else {
            assert(_systemDividerRight == el);
            _systemDividerRight = 0;
        }
        break;

    default:
        LOGD("System::remove(%s) not implemented", el->typeName());
        return;
    }

    el->removed();
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void System::change(EngravingItem* o, EngravingItem* n)
{
    remove(o);
    add(n);
}

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction System::snap(const Fraction& tick, const PointF p) const
{
    for (const MeasureBase* m : ml) {
        if (p.x() < m->x() + m->width()) {
            return toMeasure(m)->snap(tick, p - m->pos());       //TODO: MeasureBase
        }
    }
    return toMeasure(ml.back())->snap(tick, p - pos());          //TODO: MeasureBase
}

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction System::snapNote(const Fraction& tick, const PointF p, int staff) const
{
    for (const MeasureBase* m : ml) {
        if (p.x() < m->x() + m->width()) {
            return toMeasure(m)->snapNote(tick, p - m->pos(), staff);        //TODO: MeasureBase
        }
    }
    return toMeasure(ml.back())->snap(tick, p - pos());          // TODO: MeasureBase
}

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* System::firstMeasure() const
{
    auto i = std::find_if(ml.begin(), ml.end(), [](MeasureBase* mb) { return mb->isMeasure(); });
    return i != ml.end() ? toMeasure(*i) : 0;
}

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const
{
    auto i = std::find_if(ml.rbegin(), ml.rend(), [](MeasureBase* mb) { return mb->isMeasure(); });
    return i != ml.rend() ? toMeasure(*i) : 0;
}

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

MeasureBase* System::nextMeasure(const MeasureBase* m) const
{
    if (m == ml.back()) {
        return 0;
    }
    MeasureBase* nm = m->next();
    if (nm->isMeasure() && score()->styleB(Sid::createMultiMeasureRests) && toMeasure(nm)->hasMMRest()) {
        nm = toMeasure(nm)->mmRest();
    }
    return nm;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void System::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (vbox()) {
        return;
    }
    for (Bracket* b : _brackets) {
        func(data, b);
    }

    if (_systemDividerLeft) {
        func(data, _systemDividerLeft);
    }
    if (_systemDividerRight) {
        func(data, _systemDividerRight);
    }

    int idx = 0;
    for (const SysStaff* st : _staves) {
        if (all || st->show()) {
            for (InstrumentName* t : st->instrumentNames) {
                func(data, t);
            }
        }
        ++idx;
    }
    for (SpannerSegment* ss : _spannerSegments) {
        staff_idx_t staffIdx = ss->spanner()->staffIdx();
        if (staffIdx == mu::nidx) {
            LOGD("System::scanElements: staffIDx == -1: %s %p", ss->spanner()->typeName(), ss->spanner());
            staffIdx = 0;
        }
        bool v = true;
        Spanner* spanner = ss->spanner();
        if (spanner->anchor() == Spanner::Anchor::SEGMENT || spanner->anchor() == Spanner::Anchor::CHORD) {
            EngravingItem* se = spanner->startElement();
            EngravingItem* ee = spanner->endElement();
            bool v1 = true;
            if (se && se->isChordRest()) {
                ChordRest* cr = toChordRest(se);
                Measure* m    = cr->measure();
                v1            = m->visible(cr->staffIdx());
            }
            bool v2 = true;
            if (!v1 && ee && ee->isChordRest()) {
                ChordRest* cr = toChordRest(ee);
                Measure* m    = cr->measure();
                v2            = m->visible(cr->staffIdx());
            }
            v = v1 || v2;       // hide spanner if both chords are hidden
        }
        if (all || (score()->staff(staffIdx)->show() && _staves[staffIdx]->show() && v) || spanner->isVolta() || spanner->systemFlag()) {
            ss->scanElements(data, func, all);
        }
    }
}

//---------------------------------------------------------
//   staffYpage
//    return page coordinates
//---------------------------------------------------------

double System::staffYpage(staff_idx_t staffIdx) const
{
    if (staffIdx >= _staves.size()) {
        return pagePos().y();
    }

    return _staves[staffIdx]->y() + y();
}

//---------------------------------------------------------
//   staffCanvasYpage
//    return canvas coordinates
//---------------------------------------------------------

double System::staffCanvasYpage(staff_idx_t staffIdx) const
{
    return _staves[staffIdx]->y() + y() + page()->canvasPos().y();
}

SysStaff* System::staff(size_t staffIdx) const
{
    if (staffIdx < _staves.size()) {
        return _staves[staffIdx];
    }

    return nullptr;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void System::write(XmlWriter& xml) const
{
    xml.startElement(this);
    if (_systemDividerLeft && _systemDividerLeft->isUserModified()) {
        _systemDividerLeft->write(xml);
    }
    if (_systemDividerRight && _systemDividerRight->isUserModified()) {
        _systemDividerRight->write(xml);
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void System::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "SystemDivider") {
            SystemDivider* sd = new SystemDivider(this);
            sd->read(e);
            add(sd);
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* System::nextSegmentElement()
{
    Measure* m = firstMeasure();
    if (m) {
        Segment* firstSeg = m->segments().first();
        if (firstSeg) {
            return firstSeg->element(0);
        }
    }
    return score()->lastElement();
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* System::prevSegmentElement()
{
    Segment* seg = firstMeasure()->first();
    EngravingItem* re = 0;
    while (!re) {
        seg = seg->prev1MM();
        if (!seg) {
            return score()->firstElement();
        }

        if (seg->segmentType() == SegmentType::EndBarLine) {
            score()->inputState().setTrack((score()->staves().size() - 1) * VOICES);       //correction
        }
        re = seg->lastElement(score()->staves().size() - 1);
    }
    return re;
}

//---------------------------------------------------------
//   minDistance
//    Return the minimum distance between this system and s2
//    without any element collisions.
//
//    this - top system
//    s2   - bottom system
//---------------------------------------------------------

double System::minDistance(System* s2) const
{
    if (vbox() && !s2->vbox()) {
        return std::max(double(vbox()->bottomGap()), s2->minTop());
    } else if (!vbox() && s2->vbox()) {
        return std::max(double(s2->vbox()->topGap()), minBottom());
    } else if (vbox() && s2->vbox()) {
        return double(s2->vbox()->topGap() + vbox()->bottomGap());
    }

    if (_staves.empty() || s2->staves().empty()) {
        return 0.0;
    }

    double minVerticalDistance = score()->styleMM(Sid::minVerticalDistance);
    double dist = score()->enableVerticalSpread() ? styleP(Sid::minSystemSpread) : styleP(Sid::minSystemDistance);
    size_t firstStaff = 0;
    size_t lastStaff = 0;

    for (firstStaff = 0; firstStaff < _staves.size() - 1; ++firstStaff) {
        if (score()->staff(firstStaff)->show() && s2->staff(firstStaff)->show()) {
            break;
        }
    }
    for (lastStaff = _staves.size() - 1; lastStaff > 0; --lastStaff) {
        if (score()->staff(lastStaff)->show() && staff(lastStaff)->show()) {
            break;
        }
    }

    Staff* staff = score()->staff(firstStaff);
    double userDist = staff ? staff->userDist() : 0.0;
    dist = std::max(dist, userDist);
    fixedDownDistance = false;

    for (MeasureBase* mb1 : ml) {
        if (mb1->isMeasure()) {
            Measure* m = toMeasure(mb1);
            Spacer* sp = m->vspacerDown(lastStaff);
            if (sp) {
                if (sp->spacerType() == SpacerType::FIXED) {
                    dist = sp->gap();
                    fixedDownDistance = true;
                    break;
                } else {
                    dist = std::max(dist, sp->gap().val());
                }
            }
        }
    }
    if (!fixedDownDistance) {
        for (MeasureBase* mb2 : s2->ml) {
            if (mb2->isMeasure()) {
                Measure* m = toMeasure(mb2);
                Spacer* sp = m->vspacerUp(firstStaff);
                if (sp) {
                    dist = std::max(dist, sp->gap().val());
                }
            }
        }

        SysStaff* sysStaff = this->staff(lastStaff);
        double sld = sysStaff ? sysStaff->skyline().minDistance(s2->staff(firstStaff)->skyline()) : 0;
        sld -= sysStaff ? sysStaff->bbox().height() - minVerticalDistance : 0;
        dist = std::max(dist, sld);
    }
    return dist;
}

//---------------------------------------------------------
//   topDistance
//    return minimum distance to the above south skyline
//---------------------------------------------------------

double System::topDistance(staff_idx_t staffIdx, const SkylineLine& s) const
{
    assert(!vbox());
    assert(!s.isNorth());
    // in continuous view, we only build a partial skyline for performance reasons
    // this means we cannot expect the minDistance calculation to produce meaningful results
    // so just give up on autoplace for spanners in continuous view
    // (or any other calculations that rely on this value)
    if (score()->lineMode()) {
        return 0.0;
    }
    return s.minDistance(staff(staffIdx)->skyline().north());
}

//---------------------------------------------------------
//   bottomDistance
//---------------------------------------------------------

double System::bottomDistance(staff_idx_t staffIdx, const SkylineLine& s) const
{
    assert(!vbox());
    assert(s.isNorth());
    // see note on topDistance() above
    if (score()->lineMode()) {
        return 0.0;
    }
    return staff(staffIdx)->skyline().south().minDistance(s);
}

//---------------------------------------------------------
//   firstVisibleSysStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleSysStaff() const
{
    size_t nstaves = _staves.size();
    for (staff_idx_t i = 0; i < nstaves; ++i) {
        if (_staves[i]->show()) {
            return i;
        }
    }
    return mu::nidx;
}

//---------------------------------------------------------
//   lastVisibleSysStaff
//---------------------------------------------------------

staff_idx_t System::lastVisibleSysStaff() const
{
    int nstaves = static_cast<int>(_staves.size());
    for (int i = nstaves - 1; i >= 0; --i) {
        if (_staves[i]->show()) {
            return static_cast<staff_idx_t>(i);
        }
    }
    return mu::nidx;
}

//---------------------------------------------------------
//   minTop
//    Return the minimum top margin.
//---------------------------------------------------------

double System::minTop() const
{
    staff_idx_t si = firstVisibleSysStaff();
    SysStaff* s = si == mu::nidx ? nullptr : staff(si);
    if (s) {
        return -s->skyline().north().max();
    }
    return 0.0;
}

//---------------------------------------------------------
//   minBottom
//    Return the minimum bottom margin.
//---------------------------------------------------------

double System::minBottom() const
{
    if (vbox()) {
        return vbox()->bottomGap();
    }
    staff_idx_t si = lastVisibleSysStaff();
    SysStaff* s = si == mu::nidx ? nullptr : staff(si);
    if (s) {
        return s->skyline().south().max() - s->bbox().height();
    }
    return 0.0;
}

//---------------------------------------------------------
//   spacerDistance
//    Return the distance needed due to spacers
//---------------------------------------------------------

double System::spacerDistance(bool up) const
{
    staff_idx_t staff = up ? firstVisibleSysStaff() : lastVisibleSysStaff();
    if (staff == mu::nidx) {
        return 0.0;
    }
    double dist = 0.0;
    for (MeasureBase* mb : measures()) {
        if (mb->isMeasure()) {
            Measure* m = toMeasure(mb);
            Spacer* sp = up ? m->vspacerUp(staff) : m->vspacerDown(staff);
            if (sp) {
                if (sp->spacerType() == SpacerType::FIXED) {
                    dist = sp->gap();
                    break;
                } else {
                    dist = std::max(dist, sp->gap().val());
                }
            }
        }
    }
    return dist;
}

//---------------------------------------------------------
//   upSpacer
//    Return largest upSpacer for this system. This can
//    be a downSpacer of the previous system.
//---------------------------------------------------------

Spacer* System::upSpacer(staff_idx_t staffIdx, Spacer* prevDownSpacer) const
{
    if (staffIdx == mu::nidx) {
        return nullptr;
    }

    if (prevDownSpacer && (prevDownSpacer->spacerType() == SpacerType::FIXED)) {
        return prevDownSpacer;
    }

    Spacer* spacer { prevDownSpacer };
    for (MeasureBase* mb : measures()) {
        if (!(mb && mb->isMeasure())) {
            continue;
        }
        Spacer* sp { toMeasure(mb)->vspacerUp(staffIdx) };
        if (sp) {
            if (!spacer || ((spacer->spacerType() == SpacerType::UP) && (sp->gap() > spacer->gap()))) {
                spacer = sp;
            }
            continue;
        }
    }
    return spacer;
}

//---------------------------------------------------------
//   downSpacer
//    Return the largest downSpacer for this system.
//---------------------------------------------------------

Spacer* System::downSpacer(staff_idx_t staffIdx) const
{
    if (staffIdx == mu::nidx) {
        return nullptr;
    }

    Spacer* spacer { nullptr };
    for (MeasureBase* mb : measures()) {
        if (!(mb && mb->isMeasure())) {
            continue;
        }
        Spacer* sp { toMeasure(mb)->vspacerDown(staffIdx) };
        if (sp) {
            if (sp->spacerType() == SpacerType::FIXED) {
                return sp;
            } else {
                if (!spacer || (sp->gap() > spacer->gap())) {
                    spacer = sp;
                }
            }
        }
    }
    return spacer;
}

//---------------------------------------------------------
//   firstNoteRestSegmentX
//    in System() coordinates
//    returns the position of the first note or rest,
//    or the position just after the last non-chordrest segment
//---------------------------------------------------------

double System::firstNoteRestSegmentX(bool leading)
{
    double margin = score()->styleMM(Sid::HeaderToLineStartDistance);
    for (const MeasureBase* mb : measures()) {
        if (mb->isMeasure()) {
            const Measure* measure = static_cast<const Measure*>(mb);
            for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                if (seg->isChordRestType()) {
                    double noteRestPos = seg->measure()->pos().x() + seg->pos().x();
                    if (!leading) {
                        return noteRestPos;
                    }

                    // first CR found; back up to previous segment
                    seg = seg->prevActive();
                    while (seg && seg->allElementsInvisible()) {
                        seg = seg->prevActive();
                    }
                    if (seg) {
                        // find maximum width
                        double width = 0.0;
                        size_t n = score()->nstaves();
                        for (staff_idx_t i = 0; i < n; ++i) {
                            if (!staff(i)->show()) {
                                continue;
                            }
                            EngravingItem* e = seg->element(i * VOICES);
                            if (e && e->addToSkyline()) {
                                width = std::max(width, e->pos().x() + e->bbox().right());
                            }
                        }
                        return std::min(seg->measure()->pos().x() + seg->pos().x() + width + margin, noteRestPos);
                    } else {
                        return margin;
                    }
                }
            }
        }
    }
    LOGD("firstNoteRestSegmentX: did not find segment");
    return margin;
}

//---------------------------------------------------------
//   lastNoteRestSegmentX
//    in System() coordinates
//    returns the position of the last note or rest,
//    or the position just before the first non-chordrest segment
//---------------------------------------------------------

double System::lastNoteRestSegmentX(bool trailing)
{
    double margin = score()->spatium() / 4;  // TODO: this can be parameterizable
    //for (const MeasureBase* mb : measures()) {
    for (auto measureBaseIter = measures().rbegin(); measureBaseIter != measures().rend(); measureBaseIter++) {
        if ((*measureBaseIter)->isMeasure()) {
            const Measure* measure = static_cast<const Measure*>(*measureBaseIter);
            for (const Segment* seg = measure->last(); seg; seg = seg->prev()) {
                if (seg->isChordRestType()) {
                    double noteRestPos = seg->measure()->pos().x() + seg->pos().x();
                    if (!trailing) {
                        return noteRestPos;
                    }

                    // last CR found; find next segment after this one
                    seg = seg->nextActive();
                    while (seg && seg->allElementsInvisible()) {
                        seg = seg->nextActive();
                    }
                    if (seg) {
                        return std::max(seg->measure()->pos().x() + seg->pos().x() - margin, noteRestPos);
                    } else {
                        return bbox().x() - margin;
                    }
                }
            }
        }
    }
    LOGD("lastNoteRestSegmentX: did not find segment");
    return margin;
}

//---------------------------------------------------------
//   lastChordRest
//    returns the last chordrest of a system for a particular track
//---------------------------------------------------------

ChordRest* System::lastChordRest(track_idx_t track)
{
    for (auto measureBaseIter = measures().rbegin(); measureBaseIter != measures().rend(); measureBaseIter++) {
        if ((*measureBaseIter)->isMeasure()) {
            const Measure* measure = static_cast<const Measure*>(*measureBaseIter);
            for (const Segment* seg = measure->last(); seg; seg = seg->prev()) {
                if (seg->isChordRestType()) {
                    ChordRest* cr = seg->cr(track);
                    if (cr) {
                        return cr;
                    }
                }
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   firstChordRest
//    returns the last chordrest of a system for a particular track
//---------------------------------------------------------

ChordRest* System::firstChordRest(track_idx_t track)
{
    for (const MeasureBase* mb : measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        const Measure* measure = static_cast<const Measure*>(mb);
        for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
            if (seg->isChordRestType()) {
                ChordRest* cr = seg->cr(track);
                if (cr) {
                    return cr;
                }
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   pageBreak
//---------------------------------------------------------

bool System::pageBreak() const
{
    return ml.empty() ? false : ml.back()->pageBreak();
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction System::endTick() const
{
    return measures().back()->endTick();
}

//---------------------------------------------------------
//   firstSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::firstSysStaffOfPart(const Part* part) const
{
    staff_idx_t staffIdx = 0;
    for (const Part* p : score()->parts()) {
        if (p == part) {
            return staffIdx;
        }
        staffIdx += p->nstaves();
    }
    return mu::nidx;   // Part not found.
}

//---------------------------------------------------------
//   firstVisibleSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::firstVisibleSysStaffOfPart(const Part* part) const
{
    staff_idx_t firstIdx = firstSysStaffOfPart(part);
    for (staff_idx_t idx = firstIdx; idx < firstIdx + part->nstaves(); ++idx) {
        if (staff(idx)->show()) {
            return idx;
        }
    }
    return mu::nidx;   // No visible staves on this part.
}

//---------------------------------------------------------
//   lastSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::lastSysStaffOfPart(const Part* part) const
{
    staff_idx_t firstIdx = firstSysStaffOfPart(part);
    if (firstIdx == mu::nidx) {
        return mu::nidx;     // Part not found.
    }
    return firstIdx + part->nstaves() - 1;
}

//---------------------------------------------------------
//   lastVisibleSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::lastVisibleSysStaffOfPart(const Part* part) const
{
    staff_idx_t firstStaffIdx = firstSysStaffOfPart(part);
    if (firstStaffIdx == mu::nidx) {
        return mu::nidx;
    }
    for (int idx = static_cast<int>(lastSysStaffOfPart(part)); idx >= static_cast<int>(firstStaffIdx); --idx) {
        if (staff(idx)->show()) {
            return idx;
        }
    }
    return mu::nidx;    // No visible staves on this part.
}

//---------------------------------------------------------
//      minSysTicks
//      returns the shortest note/rest in the system
//---------------------------------------------------------

Fraction System::minSysTicks() const
{
    Fraction minTicks = Fraction::max(); // Initializing the variable at an arbitrary high value.
    // In principle, it just needs to be longer than any possible note, such that the following loop
    // always correctly returns the shortest note/rest of the system.
    for (MeasureBase* mb : measures()) {
        if (mb->isMeasure()) {
            Measure* m = toMeasure(mb);
            minTicks = std::min(m->shortestChordRest(), minTicks);
        }
    }
    return minTicks;
}

//---------------------------------------------------------
//    squeezableSpace
//    Collects the squeezable space of a system. This allows
//    for some systems to be justified by squeezing rather
//    than stretching.
//---------------------------------------------------------

double System::squeezableSpace() const
{
    double squeezableSpace = 0;
    for (auto m : measures()) {
        if (m->isMeasure()) {
            squeezableSpace += toMeasure(m)->squeezableSpace();
        }
    }
    return squeezableSpace;
}

Fraction System::maxSysTicks() const
{
    Fraction maxTicks = Fraction(0, 1);
    for (auto mb : measures()) {
        if (mb->isMeasure()) {
            maxTicks = std::max(maxTicks, toMeasure(mb)->maxTicks());
        }
    }
    return maxTicks;
}

bool System::hasCrossStaffOrModifiedBeams()
{
    for (MeasureBase* mb : measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* e : seg.elist()) {
                if (!e || !e->isChordRest()) {
                    continue;
                }
                if (toChordRest(e)->beam() && (toChordRest(e)->beam()->cross() || toChordRest(e)->beam()->userModified())) {
                    return true;
                }
                if (e->isChord() && !toChord(e)->graceNotes().empty()) {
                    for (Chord* grace : toChord(e)->graceNotes()) {
                        if (grace->beam() && (grace->beam()->cross() || grace->beam()->userModified())) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
}
