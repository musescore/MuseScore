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

#include <QtMath>

#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/spacer.h"
#include "libmscore/score.h"

#include "realfn.h"

using namespace mu::engraving;
using namespace Ms;

VerticalGapData::VerticalGapData(bool first, System* sys, Staff* st, SysStaff* sst, Spacer* nextSpacer, qreal y)
    : _fixedHeight(first), system(sys), sysStaff(sst), staff(st)
{
    if (_fixedHeight) {
        _normalisedSpacing = system->score()->styleP(Sid::staffUpperBorder);
        _maxActualSpacing = _normalisedSpacing;
    } else {
        _normalisedSpacing = system->y() + (sysStaff ? sysStaff->bbox().y() : 0.0) - y;
        _maxActualSpacing = system->score()->styleP(Sid::maxStaffSpread);

        Spacer* spacer { staff ? system->upSpacer(staff->idx(), nextSpacer) : nullptr };

        if (spacer) {
            _fixedSpacer = spacer->spacerType() == SpacerType::FIXED;
            _normalisedSpacing = qMax(_normalisedSpacing, spacer->gap());
            if (_fixedSpacer) {
                _maxActualSpacing = _normalisedSpacing;
            }
        }
    }
}

//---------------------------------------------------------
//   updateFactor
//---------------------------------------------------------

void VerticalGapData::updateFactor(qreal factor)
{
    if (_fixedHeight) {
        return;
    }
    qreal f = qMax(factor, _factor);
    _normalisedSpacing *= _factor / f;
    _factor = f;
}

//---------------------------------------------------------
//   addSpaceBetweenSections
//---------------------------------------------------------

void VerticalGapData::addSpaceBetweenSections()
{
    updateFactor(system->score()->styleD(Sid::spreadSystem));
    if (!(_fixedHeight | _fixedSpacer)) {
        _maxActualSpacing = system->score()->styleP(Sid::maxSystemSpread) / _factor;
    }
}

//---------------------------------------------------------
//   addSpaceAroundVBox
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundVBox(bool above)
{
    _fixedHeight = true;
    _factor = 1.0;
    const Score* score { system->score() };
    _normalisedSpacing = above ? score->styleP(Sid::frameSystemDistance) : score->styleP(Sid::systemFrameDistance);
    _maxActualSpacing = _normalisedSpacing / _factor;
}

//---------------------------------------------------------
//   addSpaceAroundNormalBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundNormalBracket()
{
    updateFactor(system->score()->styleD(Sid::spreadSquareBracket));
}

//---------------------------------------------------------
//   addSpaceAroundCurlyBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundCurlyBracket()
{
    updateFactor(system->score()->styleD(Sid::spreadCurlyBracket));
}

//---------------------------------------------------------
//   insideCurlyBracket
//---------------------------------------------------------

void VerticalGapData::insideCurlyBracket()
{
    _maxActualSpacing = system->score()->styleP(Sid::maxAkkoladeDistance) / _factor;
}

//---------------------------------------------------------
//   factor
//---------------------------------------------------------

qreal VerticalGapData::factor() const
{
    return _factor;
}

//---------------------------------------------------------
//   spacing
//    return normalised spacing
//---------------------------------------------------------

qreal VerticalGapData::spacing() const
{
    return _normalisedSpacing + _addedNormalisedSpace;
}

//---------------------------------------------------------
//   addedSpace
//---------------------------------------------------------

qreal VerticalGapData::actualAddedSpace() const
{
    return _addedNormalisedSpace * factor();
}

//---------------------------------------------------------
//   addSpacing
//---------------------------------------------------------

qreal VerticalGapData::addSpacing(qreal step)
{
    if (_fixedHeight | _fixedSpacer) {
        return 0.0;
    }
    if (_normalisedSpacing >= _maxActualSpacing) {
        _normalisedSpacing = _maxActualSpacing;
        step = 0.0;
    } else {
        qreal newSpacing { _normalisedSpacing + _addedNormalisedSpace + step };
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

qreal VerticalGapData::addFillSpacing(qreal step, qreal maxFill)
{
    if (_fixedSpacer) {
        return 0.0;
    }
    qreal actStep { ((step + _fillSpacing / _factor) > maxFill) ? (maxFill - _fillSpacing / _factor) : step };
    qreal res = addSpacing(actStep);
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

qreal VerticalGapDataList::sumStretchFactor() const
{
    qreal sum { 0.0 };
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

qreal VerticalGapDataList::smallest(qreal limit) const
{
    VerticalGapData* vdp { nullptr };
    for (VerticalGapData* vgd : *this) {
        if (vgd->isFixedHeight()) {
            continue;
        }
        if ((qCeil(limit) == qCeil(vgd->spacing()))) {
            continue;
        }
        if (!vdp || (vgd->spacing() < vdp->spacing())) {
            vdp = vgd;
        }
    }
    return vdp ? vdp->spacing() : 0.0;
}
