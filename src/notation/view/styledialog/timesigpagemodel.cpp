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
#include "timesigpagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

TimeSigPageModel::TimeSigPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::timeSigPlacement,
                                         StyleId::timeSigNormalStyle,
                                         StyleId::timeSigAboveStyle,
                                         StyleId::timeSigAcrossStyle,
                                         StyleId::timeSigNormalScale,
                                         StyleId::timeSigAboveScale,
                                         StyleId::timeSigAcrossScale,
                                         StyleId::timeSigNormalNumDist,
                                         StyleId::timeSigAboveNumDist,
                                         StyleId::timeSigAcrossNumDist,
                                         StyleId::timeSigNormalY,
                                         StyleId::timeSigAboveY,
                                         StyleId::timeSigAcrossY,
                                         StyleId::timeSigCenterOnBarline,
                                         StyleId::timeSigHangIntoMargin,
                                         StyleId::timeSigCenterAcrossStaveGroup, })
{
}

StyleItem* TimeSigPageModel::timeSigPlacement() const
{
    return styleItem(StyleId::timeSigPlacement);
}

StyleItem* TimeSigPageModel::timeSigNormalStyle() const
{
    return styleItem(StyleId::timeSigNormalStyle);
}

StyleItem* TimeSigPageModel::timeSigAboveStyle() const
{
    return styleItem(StyleId::timeSigAboveStyle);
}

StyleItem* TimeSigPageModel::timeSigAcrossStyle() const
{
    return styleItem(StyleId::timeSigAcrossStyle);
}

StyleItem* TimeSigPageModel::timeSigNormalNumDist() const
{
    return styleItem(StyleId::timeSigNormalNumDist);
}

StyleItem* TimeSigPageModel::timeSigAboveNumDist() const
{
    return styleItem(StyleId::timeSigAboveNumDist);
}

StyleItem* TimeSigPageModel::timeSigAcrossNumDist() const
{
    return styleItem(StyleId::timeSigAcrossNumDist);
}

StyleItem* TimeSigPageModel::timeSigNormalY() const
{
    return styleItem(StyleId::timeSigNormalY);
}

StyleItem* TimeSigPageModel::timeSigAboveY() const
{
    return styleItem(StyleId::timeSigAboveY);
}

StyleItem* TimeSigPageModel::timeSigAcrossY() const
{
    return styleItem(StyleId::timeSigAcrossY);
}

StyleItem* TimeSigPageModel::timeSigCenterOnBarline() const
{
    return styleItem(StyleId::timeSigCenterOnBarline);
}

StyleItem* TimeSigPageModel::timeSigHangIntoMargin() const
{
    return styleItem(StyleId::timeSigHangIntoMargin);
}

StyleItem* TimeSigPageModel::timeSigCenterAcrossStaveGroup() const
{
    return styleItem(StyleId::timeSigCenterAcrossStaveGroup);
}

StyleItem* TimeSigPageModel::timeSigNormalScale() const
{
    return styleItem(StyleId::timeSigNormalScale);
}

StyleItem* TimeSigPageModel::timeSigAboveScale() const
{
    return styleItem(StyleId::timeSigAboveScale);
}

StyleItem* TimeSigPageModel::timeSigAcrossScale() const
{
    return styleItem(StyleId::timeSigAcrossScale);
}
