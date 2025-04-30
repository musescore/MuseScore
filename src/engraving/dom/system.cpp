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

/**
 \file
 Implementation of classes SysStaff and System.
*/

#include "system.h"

#include "style/style.h"

#include "actionicon.h"
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
#include "sig.h"
#include "spacer.h"
#include "spanner.h"
#include "staff.h"
#include "system.h"
#include "systemdivider.h"

#include "tremolotwochord.h"

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
    muse::DeleteAll(instrumentNames);
}

//---------------------------------------------------------
//   yBottom
//---------------------------------------------------------

double SysStaff::yBottom() const
{
    return skyline().south().valid() ? skyline().south().max() : m_height;
}

//---------------------------------------------------------
//   saveLayout
//---------------------------------------------------------

void SysStaff::saveLayout()
{
    m_height =  bbox().height();
    m_yPos = bbox().y();
}

//---------------------------------------------------------
//   saveLayout
//---------------------------------------------------------

void SysStaff::restoreLayout()
{
    bbox().setTop(m_yPos);
    bbox().setHeight(m_height);
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
    muse::DeleteAll(m_staves);
    muse::DeleteAll(m_brackets);
    muse::DeleteAll(m_lockIndicators);
    delete m_systemDividerLeft;
    delete m_systemDividerRight;
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
    m_ml.clear();
    for (SpannerSegment* ss : m_spannerSegments) {
        if (ss->system() == this) {
            ss->resetExplicitParent();             // assume parent() is System
        }
    }
    m_spannerSegments.clear();
    // _systemDividers are reused
}

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

void System::appendMeasure(MeasureBase* mb)
{
    assert(!mb->isMeasure() || !(style().styleB(Sid::createMultiMeasureRests) && toMeasure(mb)->hasMMRest()));
    mb->setParent(this);
    m_ml.push_back(mb);
}

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void System::removeMeasure(MeasureBase* mb)
{
    m_ml.erase(std::remove(m_ml.begin(), m_ml.end(), mb), m_ml.end());
    if (mb->system() == this) {
        mb->resetExplicitParent();
    }
}

//---------------------------------------------------------
//   removeLastMeasure
//---------------------------------------------------------

void System::removeLastMeasure()
{
    if (m_ml.empty()) {
        return;
    }
    MeasureBase* mb = m_ml.back();
    m_ml.pop_back();
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
    if (!m_ml.empty()) {
        if (m_ml[0]->isVBox() || m_ml[0]->isTBox() || m_ml[0]->isFBox()) {
            return toBox(m_ml[0]);
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
        staff->bbox().setTop(m_staves[idx - 1]->y() + 6 * spatium());
    }
    m_staves.insert(m_staves.begin() + idx, staff);
    return staff;
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void System::removeStaff(int idx)
{
    m_staves.erase(m_staves.begin() + idx);
}

//---------------------------------------------------------
//   adjustStavesNumber
//---------------------------------------------------------

void System::adjustStavesNumber(size_t nstaves)
{
    for (size_t i = m_staves.size(); i < nstaves; ++i) {
        insertStaff(static_cast<int>(i));
    }
    const size_t dn = m_staves.size() - nstaves;
    for (size_t i = 0; i < dn; ++i) {
        removeStaff(static_cast<int>(m_staves.size()) - 1);
    }
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

bool System::isLocked() const
{
    return m_ml.front()->isStartOfSystemLock();
}

const SystemLock* System::systemLock() const
{
    return m_ml.front()->systemLock();
}

void System::addLockIndicator(SystemLockIndicator* sli)
{
    assert(sli);
    m_lockIndicators.push_back(sli);
}

void System::deleteLockIndicators()
{
    muse::DeleteAll(m_lockIndicators);
    m_lockIndicators.clear();
}

void System::setBracketsXPosition(const double xPosition)
{
    for (Bracket* b1 : m_brackets) {
        BracketType bracketType = b1->bracketType();
        // For brackets that are drawn, we must correct for half line width
        double lineWidthCorrection = 0.0;
        if (bracketType == BracketType::NORMAL || bracketType == BracketType::LINE) {
            lineWidthCorrection = style().styleMM(Sid::bracketWidth) / 2;
        }
        // Compute offset cause by other stacked brackets
        double xOffset = 0;
        for (const Bracket* b2 : m_brackets) {
            if (!b2->bracketItem()->visible()) {
                continue;
            }
            bool b1FirstStaffInB2 = (b1->firstStaff() >= b2->firstStaff() && b1->firstStaff() <= b2->lastStaff());
            bool b1LastStaffInB2 = (b1->lastStaff() >= b2->firstStaff() && b1->lastStaff() <= b2->lastStaff());
            if (b1->column() > b2->column()
                && (b1FirstStaffInB2 || b1LastStaffInB2)) {
                xOffset += b2->ldata()->bracketWidth();
            }
        }
        // Set position
        double x = xPosition - xOffset - b1->ldata()->bracketWidth() + lineWidthCorrection;
        b1->mutldata()->setPosX(x);
    }
}

//---------------------------------------------------------
//   nextVisibleStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleStaffFrom(staff_idx_t startStaffIdx) const
{
    for (staff_idx_t i = startStaffIdx; i < m_staves.size(); ++i) {
        Staff* s  = score()->staff(i);
        SysStaff* ss = m_staves[i];

        if (s->show() && ss->show()) {
            return i;
        }
    }

    return muse::nidx;
}

staff_idx_t System::nextVisibleStaff(staff_idx_t staffIdx) const
{
    return firstVisibleStaffFrom(staffIdx + 1);
}

staff_idx_t System::prevVisibleStaff(staff_idx_t startStaffIdx) const
{
    if (startStaffIdx == 0) {
        return muse::nidx;
    }

    for (staff_idx_t i = startStaffIdx - 1;; --i) {
        Staff* s  = score()->staff(i);
        SysStaff* ss = m_staves[i];

        if (s->show() && ss->show()) {
            return i;
        }

        if (i == 0) {
            break;
        }
    }

    return muse::nidx;
}

//---------------------------------------------------------
//   firstVisibleStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleStaff() const
{
    return firstVisibleStaffFrom(0);
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
        m_staves[el->staffIdx()]->instrumentNames.push_back(toInstrumentName(el));
        toInstrumentName(el)->setSysStaff(m_staves[el->staffIdx()]);
        break;

    case ElementType::BEAM:
        score()->addElement(el);
        break;

    case ElementType::BRACKET: {
        Bracket* b   = toBracket(el);
        m_brackets.push_back(b);
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
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
    case ElementType::WHAMMY_BAR_SEGMENT:
    case ElementType::RASGUEADO_SEGMENT:
    case ElementType::HARMONIC_MARK_SEGMENT:
    case ElementType::PICK_SCRAPE_SEGMENT:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
    {
        SpannerSegment* ss = toSpannerSegment(el);
#ifndef NDEBUG
        if (muse::contains(m_spannerSegments, ss)) {
            LOGD("System::add() %s %p already there", ss->typeName(), ss);
        } else
#endif
        m_spannerSegments.push_back(ss);
    }
    break;

    case ElementType::SYSTEM_DIVIDER:
    {
        SystemDivider* sd = toSystemDivider(el);
        if (sd->dividerType() == SystemDivider::Type::LEFT) {
            m_systemDividerLeft = sd;
        } else {
            m_systemDividerRight = sd;
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
        muse::remove(m_staves[el->staffIdx()]->instrumentNames, toInstrumentName(el));
        toInstrumentName(el)->setSysStaff(0);
        break;
    case ElementType::BEAM:
        score()->removeElement(el);
        break;
    case ElementType::BRACKET:
    {
        Bracket* b = toBracket(el);
        if (!muse::remove(m_brackets, b)) {
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
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE_SEGMENT:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
        if (!muse::remove(m_spannerSegments, toSpannerSegment(el))) {
            LOGD("System::remove: %p(%s) not found, score %p", el, el->typeName(), score());
            assert(score() == el->score());
        }
        break;
    case ElementType::SYSTEM_DIVIDER:
        if (el == m_systemDividerLeft) {
            m_systemDividerLeft = 0;
        } else {
            assert(m_systemDividerRight == el);
            m_systemDividerRight = 0;
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
    for (const MeasureBase* m : m_ml) {
        if (p.x() < m->x() + m->width()) {
            return toMeasure(m)->snap(tick, p - m->pos());       //TODO: MeasureBase
        }
    }
    return toMeasure(m_ml.back())->snap(tick, p - pos());          //TODO: MeasureBase
}

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction System::snapNote(const Fraction& tick, const PointF p, int staff) const
{
    for (const MeasureBase* m : m_ml) {
        if (p.x() < m->x() + m->width()) {
            return toMeasure(m)->snapNote(tick, p - m->pos(), staff);        //TODO: MeasureBase
        }
    }
    return toMeasure(m_ml.back())->snap(tick, p - pos());          // TODO: MeasureBase
}

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* System::firstMeasure() const
{
    auto i = std::find_if(m_ml.begin(), m_ml.end(), [](MeasureBase* mb) { return mb->isMeasure(); });
    return i != m_ml.end() ? toMeasure(*i) : 0;
}

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const
{
    auto i = std::find_if(m_ml.rbegin(), m_ml.rend(), [](MeasureBase* mb) { return mb->isMeasure(); });
    return i != m_ml.rend() ? toMeasure(*i) : 0;
}

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

MeasureBase* System::nextMeasure(const MeasureBase* m) const
{
    if (m == m_ml.back()) {
        return 0;
    }
    MeasureBase* nm = m->next();
    if (nm->isMeasure() && style().styleB(Sid::createMultiMeasureRests) && toMeasure(nm)->hasMMRest()) {
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
    for (Bracket* b : m_brackets) {
        func(data, b);
    }

    if (m_systemDividerLeft) {
        func(data, m_systemDividerLeft);
    }
    if (m_systemDividerRight) {
        func(data, m_systemDividerRight);
    }

    for (auto i : m_lockIndicators) {
        func(data, i);
    }

    for (const SysStaff* st : m_staves) {
        if (all || st->show()) {
            for (InstrumentName* t : st->instrumentNames) {
                func(data, t);
            }
        }
    }
    for (SpannerSegment* ss : m_spannerSegments) {
        staff_idx_t staffIdx = ss->spanner()->staffIdx();
        if (staffIdx == muse::nidx) {
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
        if (all || (score()->staff(staffIdx)->show() && m_staves[staffIdx]->show() && v) || spanner->isVolta() || spanner->systemFlag()) {
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
    if (staffIdx >= m_staves.size()) {
        return pagePos().y();
    }

    return m_staves[staffIdx]->y() + y();
}

//---------------------------------------------------------
//   staffCanvasYpage
//    return canvas coordinates
//---------------------------------------------------------

double System::staffCanvasYpage(staff_idx_t staffIdx) const
{
    return m_staves[staffIdx]->y() + y() + page()->canvasPos().y();
}

SysStaff* System::staff(size_t staffIdx) const
{
    if (staffIdx < m_staves.size()) {
        return m_staves[staffIdx];
    }

    return nullptr;
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
    EngravingItem* re = 0;
    Measure* m = firstMeasure();
    if (m) {
        Segment* seg = m->first();
        while (!re) {
            seg = seg->prev1MM();
            if (!seg) {
                return score()->firstElement();
            }

            if (seg->segmentType() == SegmentType::EndBarLine) {
                score()->inputState().setTrack((score()->staves().size() - 1) * VOICES);       //correction
            }
            re = seg->lastElementForNavigation(score()->staves().size() - 1);
        }
    }
    return re;
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
    if (score()->lineMode() && !configuration()->minDistanceForPartialSkylineCalculated()) {
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
    if (score()->lineMode() && !configuration()->minDistanceForPartialSkylineCalculated()) {
        return 0.0;
    }
    return staff(staffIdx)->skyline().south().minDistance(s);
}

//---------------------------------------------------------
//   firstVisibleSysStaff
//---------------------------------------------------------

staff_idx_t System::firstVisibleSysStaff() const
{
    size_t nstaves = m_staves.size();
    for (staff_idx_t i = 0; i < nstaves; ++i) {
        if (m_staves[i]->show()) {
            return i;
        }
    }
    return muse::nidx;
}

//---------------------------------------------------------
//   lastVisibleSysStaff
//---------------------------------------------------------

staff_idx_t System::lastVisibleSysStaff() const
{
    int nstaves = static_cast<int>(m_staves.size());
    for (int i = nstaves - 1; i >= 0; --i) {
        if (m_staves[i]->show()) {
            return static_cast<staff_idx_t>(i);
        }
    }
    return muse::nidx;
}

//---------------------------------------------------------
//   minTop
//    Return the minimum top margin.
//---------------------------------------------------------

double System::minTop() const
{
    staff_idx_t si = firstVisibleSysStaff();
    SysStaff* s = si == muse::nidx ? nullptr : staff(si);
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
    if (const Box* vb = vbox()) {
        return vb->absoluteFromSpatium(vb->bottomGap());
    }
    staff_idx_t si = lastVisibleSysStaff();
    SysStaff* s = si == muse::nidx ? nullptr : staff(si);
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
    if (staff == muse::nidx) {
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
    if (staffIdx == muse::nidx) {
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
    if (staffIdx == muse::nidx) {
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

double System::firstNoteRestSegmentX(bool leading) const
{
    double margin = style().styleMM(Sid::headerToLineStartDistance);
    for (const MeasureBase* mb : measures()) {
        if (mb->isMeasure()) {
            const Measure* measure = static_cast<const Measure*>(mb);
            margin = measure->firstNoteRestSegmentX(leading);
            break;
        }
    }

    return margin;
}

//---------------------------------------------------------
//   endingXForOpenEndedLines
//    in System() coordinates returns the end point for
//    open ended (i.e. cross-system) lines
//---------------------------------------------------------

double System::endingXForOpenEndedLines() const
{
    double margin = style().styleMM(Sid::lineEndToBarlineDistance);
    double systemEndX = ldata()->bbox().width();

    Measure* lastMeas = lastMeasure();
    if (!lastMeas) {
        return systemEndX - margin;
    }

    return lastMeas->endingXForOpenEndedLines();
}

//---------------------------------------------------------
//   lastChordRest
//    returns the last chordrest of a system for a particular track
//---------------------------------------------------------

ChordRest* System::lastChordRest(track_idx_t track) const
{
    for (auto measureBaseIter = measures().rbegin(); measureBaseIter != measures().rend(); measureBaseIter++) {
        if ((*measureBaseIter)->isMeasure()) {
            const Measure* measure = static_cast<const Measure*>(*measureBaseIter);
            return measure->lastChordRest(track);
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   firstChordRest
//    returns the last chordrest of a system for a particular track
//---------------------------------------------------------

ChordRest* System::firstChordRest(track_idx_t track) const
{
    for (const MeasureBase* mb : measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        const Measure* measure = static_cast<const Measure*>(mb);
        return measure->firstChordRest(track);
    }
    return nullptr;
}

//---------------------------------------------------------
//   pageBreak
//---------------------------------------------------------

bool System::pageBreak() const
{
    return m_ml.empty() ? false : m_ml.back()->pageBreak();
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
    return muse::nidx;   // Part not found.
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
    return muse::nidx;   // No visible staves on this part.
}

//---------------------------------------------------------
//   lastSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::lastSysStaffOfPart(const Part* part) const
{
    staff_idx_t firstIdx = firstSysStaffOfPart(part);
    if (firstIdx == muse::nidx) {
        return muse::nidx;     // Part not found.
    }
    return firstIdx + part->nstaves() - 1;
}

//---------------------------------------------------------
//   lastVisibleSysStaffOfPart
//---------------------------------------------------------

staff_idx_t System::lastVisibleSysStaffOfPart(const Part* part) const
{
    staff_idx_t firstStaffIdx = firstSysStaffOfPart(part);
    if (firstStaffIdx == muse::nidx) {
        return muse::nidx;
    }
    for (int idx = static_cast<int>(lastSysStaffOfPart(part)); idx >= static_cast<int>(firstStaffIdx); --idx) {
        if (staff(idx)->show()) {
            return idx;
        }
    }
    return muse::nidx;    // No visible staves on this part.
}
}
