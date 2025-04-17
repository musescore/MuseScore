/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "verticalgapdata.h"

#include "realfn.h"

#include "dom/spacer.h"
#include "dom/staff.h"
#include "dom/system.h"

#include "style/style.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

VerticalGapData::VerticalGapData(const MStyle* style, bool first, System* sys, const Staff* st, SysStaff* sst, Spacer* nextSpacer, double y)
    : style(style), system(sys), sysStaff(sst), staff(st), m_fixedHeight(first)
{
    if (m_fixedHeight) {
        m_normalisedSpacing = style->styleMM(Sid::staffUpperBorder);
        m_maxActualSpacing = m_normalisedSpacing;
    } else {
        m_normalisedSpacing = system->y() + (sysStaff ? sysStaff->bbox().y() : 0.0) - y;
        m_maxActualSpacing = style->styleMM(Sid::maxStaffSpread);

        Spacer* spacer { staff ? system->upSpacer(staff->idx(), nextSpacer) : nullptr };

        if (spacer) {
            m_fixedSpacer = spacer->spacerType() == SpacerType::FIXED;
            m_normalisedSpacing = std::max(m_normalisedSpacing, spacer->absoluteGap());
            if (m_fixedSpacer) {
                m_maxActualSpacing = m_normalisedSpacing;
            }
        }
    }
}

//---------------------------------------------------------
//   updateFactor
//---------------------------------------------------------

void VerticalGapData::updateFactor(double factor)
{
    if (m_fixedHeight) {
        return;
    }
    double f = std::max(factor, m_factor);
    m_normalisedSpacing *= m_factor / f;
    m_factor = f;
}

//---------------------------------------------------------
//   addSpaceBetweenSections
//---------------------------------------------------------

void VerticalGapData::addSpaceBetweenSections()
{
    updateFactor(style->styleD(Sid::spreadSystem));
    if (!(m_fixedHeight | m_fixedSpacer)) {
        m_maxActualSpacing = style->styleMM(Sid::maxSystemSpread) / m_factor;
    }
}

//---------------------------------------------------------
//   addSpaceAroundVBox
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundVBox(bool above)
{
    m_fixedHeight = true;
    m_factor = 1.0;
    m_normalisedSpacing = above ? style->styleMM(Sid::frameSystemDistance) : style->styleMM(Sid::systemFrameDistance);
    m_maxActualSpacing = m_normalisedSpacing / m_factor;
}

//---------------------------------------------------------
//   addSpaceAroundNormalBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundNormalBracket()
{
    updateFactor(style->styleD(Sid::spreadSquareBracket));
}

//---------------------------------------------------------
//   addSpaceAroundCurlyBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundCurlyBracket()
{
    updateFactor(style->styleD(Sid::spreadCurlyBracket));
}

//---------------------------------------------------------
//   insideCurlyBracket
//---------------------------------------------------------

void VerticalGapData::insideCurlyBracket()
{
    m_maxActualSpacing = style->styleMM(Sid::maxAkkoladeDistance) / m_factor;
}

//---------------------------------------------------------
//   factor
//---------------------------------------------------------

double VerticalGapData::factor() const
{
    return m_factor;
}

//---------------------------------------------------------
//   spacing
//    return normalised spacing
//---------------------------------------------------------

double VerticalGapData::spacing() const
{
    return m_normalisedSpacing + m_addedNormalisedSpace;
}

//---------------------------------------------------------
//   addedSpace
//---------------------------------------------------------

double VerticalGapData::actualAddedSpace() const
{
    return m_addedNormalisedSpace * factor();
}

//---------------------------------------------------------
//   addSpacing
//---------------------------------------------------------

double VerticalGapData::addSpacing(double step)
{
    if (m_fixedHeight | m_fixedSpacer) {
        return 0.0;
    }
    if (m_normalisedSpacing >= m_maxActualSpacing) {
        m_normalisedSpacing = m_maxActualSpacing;
        step = 0.0;
    } else {
        double newSpacing { m_normalisedSpacing + m_addedNormalisedSpace + step };
        if ((newSpacing >= m_maxActualSpacing)) {
            step = m_maxActualSpacing - m_normalisedSpacing - m_addedNormalisedSpace;
        }
    }
    m_addedNormalisedSpace += step;
    m_lastStep = step;
    return step;
}

//---------------------------------------------------------
//   isFixedHeight
//---------------------------------------------------------

bool VerticalGapData::isFixedHeight() const
{
    return m_fixedHeight || muse::RealIsNull(m_normalisedSpacing - m_maxActualSpacing);
}

//---------------------------------------------------------
//   undoLastAddSpacing
//---------------------------------------------------------

void VerticalGapData::undoLastAddSpacing()
{
    m_addedNormalisedSpace -= m_lastStep;
    m_lastStep = 0.0;
}

//---------------------------------------------------------
//   addFillSpacing
//---------------------------------------------------------

double VerticalGapData::addFillSpacing(double step, double maxFill)
{
    if (m_fixedSpacer) {
        return 0.0;
    }
    double actStep { ((step + m_fillSpacing / m_factor) > maxFill) ? (maxFill - m_fillSpacing / m_factor) : step };
    double res = addSpacing(actStep);
    m_fillSpacing += res * m_factor;
    return res;
}

void VerticalGapData::setNormalisedSpacing(double newNormalisedSpacing)
{
    m_normalisedSpacing = newNormalisedSpacing;
}

//---------------------------------------------------------
//   deleteAll
//---------------------------------------------------------

void VerticalGapDataList::deleteAll()
{
    for (auto vsd : *this) {
        delete vsd;
    }
}

//---------------------------------------------------------
//   sumStretchFactor
//---------------------------------------------------------

double VerticalGapDataList::sumStretchFactor() const
{
    double sum { 0.0 };
    for (VerticalGapData* vsd : *this) {
        if (!vsd->isFixedHeight()) {
            sum += vsd->factor();
        }
    }
    return sum;
}

//---------------------------------------------------------
//   smallest
//---------------------------------------------------------

double VerticalGapDataList::smallest(double limit) const
{
    VerticalGapData* vdp { nullptr };
    for (VerticalGapData* vgd : *this) {
        if (vgd->isFixedHeight()) {
            continue;
        }
        if ((std::ceil(limit) == std::ceil(vgd->spacing()))) {
            continue;
        }
        if (!vdp || (vgd->spacing() < vdp->spacing())) {
            vdp = vgd;
        }
    }
    return vdp ? vdp->spacing() : 0.0;
}
