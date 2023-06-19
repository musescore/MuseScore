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
#include "verticalgapdata.h"

#include "realfn.h"

#include "libmscore/spacer.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"

#include "style/style.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

VerticalGapData::VerticalGapData(MStyle* style, bool first, System* sys, Staff* st, SysStaff* sst, Spacer* nextSpacer, double y)
    : _fixedHeight(first), style(style), system(sys), sysStaff(sst), staff(st)
{
    if (_fixedHeight) {
        _normalisedSpacing = style->styleMM(Sid::staffUpperBorder);
        _maxActualSpacing = _normalisedSpacing;
    } else {
        _normalisedSpacing = system->y() + (sysStaff ? sysStaff->bbox().y() : 0.0) - y;
        _maxActualSpacing = style->styleMM(Sid::maxStaffSpread);

        Spacer* spacer { staff ? system->upSpacer(staff->idx(), nextSpacer) : nullptr };

        if (spacer) {
            _fixedSpacer = spacer->spacerType() == SpacerType::FIXED;
            _normalisedSpacing = std::max(_normalisedSpacing, spacer->gap().val());
            if (_fixedSpacer) {
                _maxActualSpacing = _normalisedSpacing;
            }
        }
    }
}

//---------------------------------------------------------
//   updateFactor
//---------------------------------------------------------

void VerticalGapData::updateFactor(double factor)
{
    if (_fixedHeight) {
        return;
    }
    double f = std::max(factor, _factor);
    _normalisedSpacing *= _factor / f;
    _factor = f;
}

//---------------------------------------------------------
//   addSpaceBetweenSections
//---------------------------------------------------------

void VerticalGapData::addSpaceBetweenSections()
{
    updateFactor(style->styleD(Sid::spreadSystem));
    if (!(_fixedHeight | _fixedSpacer)) {
        _maxActualSpacing = style->styleMM(Sid::maxSystemSpread) / _factor;
    }
}

//---------------------------------------------------------
//   addSpaceAroundVBox
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundVBox(bool above)
{
    _fixedHeight = true;
    _factor = 1.0;
    _normalisedSpacing = above ? style->styleMM(Sid::frameSystemDistance) : style->styleMM(Sid::systemFrameDistance);
    _maxActualSpacing = _normalisedSpacing / _factor;
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
    _maxActualSpacing = style->styleMM(Sid::maxAkkoladeDistance) / _factor;
}

//---------------------------------------------------------
//   factor
//---------------------------------------------------------

double VerticalGapData::factor() const
{
    return _factor;
}

//---------------------------------------------------------
//   spacing
//    return normalised spacing
//---------------------------------------------------------

double VerticalGapData::spacing() const
{
    return _normalisedSpacing + _addedNormalisedSpace;
}

//---------------------------------------------------------
//   addedSpace
//---------------------------------------------------------

double VerticalGapData::actualAddedSpace() const
{
    return _addedNormalisedSpace * factor();
}

//---------------------------------------------------------
//   addSpacing
//---------------------------------------------------------

double VerticalGapData::addSpacing(double step)
{
    if (_fixedHeight | _fixedSpacer) {
        return 0.0;
    }
    if (_normalisedSpacing >= _maxActualSpacing) {
        _normalisedSpacing = _maxActualSpacing;
        step = 0.0;
    } else {
        double newSpacing { _normalisedSpacing + _addedNormalisedSpace + step };
        if ((newSpacing >= _maxActualSpacing)) {
            step = _maxActualSpacing - _normalisedSpacing - _addedNormalisedSpace;
        }
    }
    _addedNormalisedSpace += step;
    _lastStep = step;
    return step;
}

//---------------------------------------------------------
//   isFixedHeight
//---------------------------------------------------------

bool VerticalGapData::isFixedHeight() const
{
    return _fixedHeight || RealIsNull(_normalisedSpacing - _maxActualSpacing);
}

//---------------------------------------------------------
//   undoLastAddSpacing
//---------------------------------------------------------

void VerticalGapData::undoLastAddSpacing()
{
    _addedNormalisedSpace -= _lastStep;
    _lastStep = 0.0;
}

//---------------------------------------------------------
//   addFillSpacing
//---------------------------------------------------------

double VerticalGapData::addFillSpacing(double step, double maxFill)
{
    if (_fixedSpacer) {
        return 0.0;
    }
    double actStep { ((step + _fillSpacing / _factor) > maxFill) ? (maxFill - _fillSpacing / _factor) : step };
    double res = addSpacing(actStep);
    _fillSpacing += res * _factor;
    return res;
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
