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
#ifndef MU_ENGRAVING_VERTICALGAPDATALIST_H
#define MU_ENGRAVING_VERTICALGAPDATALIST_H

#include <QList>

namespace Ms {
class System;
class SysStaff;
class Staff;
class Spacer;
}

namespace mu::engraving {
//---------------------------------------------------------
//   VerticalStretchData
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapData
{
private:
    bool _fixedHeight        { false };
    bool _fixedSpacer        { false };
    qreal _factor               { 1.0 };
    qreal _normalisedSpacing    { 0.0 };
    qreal _maxActualSpacing     { 0.0 };
    qreal _addedNormalisedSpace { 0.0 };
    qreal _fillSpacing          { 0.0 };
    qreal _lastStep             { 0.0 };
    void  updateFactor(qreal factor);

public:
    Ms::System* system   { nullptr };
    Ms::SysStaff* sysStaff { nullptr };
    Ms::Staff* staff    { nullptr };

    VerticalGapData(bool first, Ms::System* sys, Ms::Staff* st, Ms::SysStaff* sst, Ms::Spacer* nextSpacer, qreal y);

    void addSpaceBetweenSections();
    void addSpaceAroundVBox(bool above);
    void addSpaceAroundNormalBracket();
    void addSpaceAroundCurlyBracket();
    void insideCurlyBracket();

    qreal factor() const;
    qreal spacing() const;
    qreal actualAddedSpace() const;

    qreal addSpacing(qreal step);
    bool isFixedHeight() const;
    void undoLastAddSpacing();
    qreal addFillSpacing(qreal step, qreal maxFill);
};

//---------------------------------------------------------
//   VerticalStretchDataList
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapDataList : public QList<VerticalGapData*>
{
public:
    void deleteAll();
    qreal sumStretchFactor() const;
    qreal smallest(qreal limit=-1.0) const;
};
}

#endif // MU_ENGRAVING_VERTICALGAPDATALIST_H
