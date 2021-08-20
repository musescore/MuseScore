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

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <set>
#include <QList>

#include "system.h"

namespace Ms {
class Segment;
class Page;

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
    System* system   { nullptr };
    SysStaff* sysStaff { nullptr };
    Staff* staff    { nullptr };

    VerticalGapData(bool first, System* sys, Staff* st, SysStaff* sst, Spacer* nextSpacer, qreal y);

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

//---------------------------------------------------------
//   VerticalAlignRange
//---------------------------------------------------------

enum class VerticalAlignRange {
    SEGMENT, MEASURE, SYSTEM
};

extern bool notTopBeam(ChordRest* cr);
extern bool notTopTuplet(ChordRest* cr);
}     // namespace Ms
#endif
