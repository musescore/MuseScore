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
#include "clefkeytimesigpagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

ClefKeyTimeSigPageModel::ClefKeyTimeSigPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::genClef,
                                         StyleId::hideTabClefAfterFirst,
                                         StyleId::genCourtesyClef,
                                         StyleId::tabClef,
                                         StyleId::timeSigPlacement,
                                         StyleId::timeSigNormalStyle,
                                         StyleId::timeSigAboveStyle,
                                         StyleId::timeSigAcrossStyle,
                                         StyleId::timeSigNormalScale,
                                         StyleId::timeSigAboveScale,
                                         StyleId::timeSigAcrossScale,
                                         StyleId::timeSigNormalScaleLock,
                                         StyleId::timeSigAboveScaleLock,
                                         StyleId::timeSigAcrossScaleLock,
                                         StyleId::timeSigNormalNumDist,
                                         StyleId::timeSigAboveNumDist,
                                         StyleId::timeSigAcrossNumDist,
                                         StyleId::timeSigNormalY,
                                         StyleId::timeSigAboveY,
                                         StyleId::timeSigAcrossY,
                                         StyleId::timeSigCenterOnBarline,
                                         StyleId::timeSigVSMarginCentered,
                                         StyleId::timeSigVSMarginNonCentered,
                                         StyleId::timeSigCenterAcrossStaveGroup,
                                         StyleId::genCourtesyTimesig,
                                         StyleId::genKeysig,
                                         StyleId::genCourtesyKeysig,
                                         StyleId::changesBeforeBarlineRepeats,
                                         StyleId::changesBeforeBarlineOtherJumps,
                                         StyleId::placeClefsBeforeRepeats,
                                         StyleId::changesBetweenEndStartRepeat,
                                         StyleId::showCourtesiesRepeats,
                                         StyleId::useParensRepeatCourtesies,
                                         StyleId::showCourtesiesOtherJumps,
                                         StyleId::useParensOtherJumpCourtesies,
                                         StyleId::showCourtesiesAfterCancellingRepeats,
                                         StyleId::useParensRepeatCourtesiesAfterCancelling,
                                         StyleId::showCourtesiesAfterCancellingOtherJumps,
                                         StyleId::useParensOtherJumpCourtesiesAfterCancelling,
                                         StyleId::smallParens })
{
}

StyleItem* ClefKeyTimeSigPageModel::genClef() const
{
    return styleItem(StyleId::genClef);
}

StyleItem* ClefKeyTimeSigPageModel::hideTabClefAfterFirst() const
{
    return styleItem(StyleId::hideTabClefAfterFirst);
}

StyleItem* ClefKeyTimeSigPageModel::genCourtesyClef() const
{
    return styleItem(StyleId::genCourtesyClef);
}

StyleItem* ClefKeyTimeSigPageModel::tabClef() const
{
    return styleItem(StyleId::tabClef);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigPlacement() const
{
    return styleItem(StyleId::timeSigPlacement);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigNormalStyle() const
{
    return styleItem(StyleId::timeSigNormalStyle);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAboveStyle() const
{
    return styleItem(StyleId::timeSigAboveStyle);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAcrossStyle() const
{
    return styleItem(StyleId::timeSigAcrossStyle);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigNormalNumDist() const
{
    return styleItem(StyleId::timeSigNormalNumDist);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAboveNumDist() const
{
    return styleItem(StyleId::timeSigAboveNumDist);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAcrossNumDist() const
{
    return styleItem(StyleId::timeSigAcrossNumDist);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigNormalY() const
{
    return styleItem(StyleId::timeSigNormalY);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAboveY() const
{
    return styleItem(StyleId::timeSigAboveY);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAcrossY() const
{
    return styleItem(StyleId::timeSigAcrossY);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigCenterOnBarline() const
{
    return styleItem(StyleId::timeSigCenterOnBarline);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigVSMarginCentered() const
{
    return styleItem(StyleId::timeSigVSMarginCentered);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigVSMarginNonCentered() const
{
    return styleItem(StyleId::timeSigVSMarginNonCentered);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigCenterAcrossStaveGroup() const
{
    return styleItem(StyleId::timeSigCenterAcrossStaveGroup);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigNormalScale() const
{
    return styleItem(StyleId::timeSigNormalScale);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAboveScale() const
{
    return styleItem(StyleId::timeSigAboveScale);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAcrossScale() const
{
    return styleItem(StyleId::timeSigAcrossScale);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigNormalScaleLock() const
{
    return styleItem(StyleId::timeSigNormalScaleLock);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAboveScaleLock() const
{
    return styleItem(StyleId::timeSigAboveScaleLock);
}

StyleItem* ClefKeyTimeSigPageModel::timeSigAcrossScaleLock() const
{
    return styleItem(StyleId::timeSigAcrossScaleLock);
}

void ClefKeyTimeSigPageModel::resetStyleAndSize() const
{
    static const std::vector<StyleId> NORMAL_IDS = {
        StyleId::timeSigNormalStyle,
        StyleId::timeSigNormalScale,
        StyleId::timeSigNormalScaleLock,
        StyleId::timeSigNormalNumDist,
        StyleId::timeSigNormalY,
    };

    static const std::vector<StyleId> ABOVE_IDS = {
        StyleId::timeSigAboveStyle,
        StyleId::timeSigAboveScale,
        StyleId::timeSigAboveScaleLock,
        StyleId::timeSigAboveNumDist,
        StyleId::timeSigAboveY,
        StyleId::timeSigCenterOnBarline,
        StyleId::timeSigVSMarginCentered,
        StyleId::timeSigVSMarginNonCentered,
    };

    static const std::vector<StyleId> ACROSS_IDS = {
        StyleId::timeSigAcrossStyle,
        StyleId::timeSigAcrossScale,
        StyleId::timeSigAcrossScaleLock,
        StyleId::timeSigAcrossNumDist,
        StyleId::timeSigAcrossY,
        StyleId::timeSigCenterAcrossStaveGroup
    };

    switch (styleItem(StyleId::timeSigPlacement)->value().value<TimeSigPlacement>()) {
    case TimeSigPlacement::NORMAL: return currentNotationStyle()->resetStyleValues(NORMAL_IDS);
    case TimeSigPlacement::ABOVE_STAVES: return currentNotationStyle()->resetStyleValues(ABOVE_IDS);
    case TimeSigPlacement::ACROSS_STAVES: return currentNotationStyle()->resetStyleValues(ACROSS_IDS);
    default:
        return;
    }
}

StyleItem* ClefKeyTimeSigPageModel::genCourtesyTimesig() const
{
    return styleItem(StyleId::genCourtesyTimesig);
}

StyleItem* ClefKeyTimeSigPageModel::genKeysig() const
{
    return styleItem(StyleId::genKeysig);
}

StyleItem* ClefKeyTimeSigPageModel::genCourtesyKeysig() const
{
    return styleItem(StyleId::genCourtesyKeysig);
}

StyleItem* ClefKeyTimeSigPageModel::changesBeforeBarlineRepeats() const
{
    return styleItem(StyleId::changesBeforeBarlineRepeats);
}

StyleItem* ClefKeyTimeSigPageModel::changesBeforeBarlineOtherJumps() const
{
    return styleItem(StyleId::changesBeforeBarlineOtherJumps);
}

StyleItem* ClefKeyTimeSigPageModel::placeClefsBeforeRepeats() const
{
    return styleItem(StyleId::placeClefsBeforeRepeats);
}

StyleItem* ClefKeyTimeSigPageModel::changesBetweenEndStartRepeat() const
{
    return styleItem(StyleId::changesBetweenEndStartRepeat);
}

StyleItem* ClefKeyTimeSigPageModel::showCourtesiesRepeats() const
{
    return styleItem(StyleId::showCourtesiesRepeats);
}

StyleItem* ClefKeyTimeSigPageModel::useParensRepeatCourtesies() const
{
    return styleItem(StyleId::useParensRepeatCourtesies);
}

StyleItem* ClefKeyTimeSigPageModel::showCourtesiesOtherJumps() const
{
    return styleItem(StyleId::showCourtesiesOtherJumps);
}

StyleItem* ClefKeyTimeSigPageModel::useParensOtherJumpCourtesies() const
{
    return styleItem(StyleId::useParensOtherJumpCourtesies);
}

StyleItem* ClefKeyTimeSigPageModel::showCourtesiesAfterCancellingRepeats() const
{
    return styleItem(StyleId::showCourtesiesAfterCancellingRepeats);
}

StyleItem* ClefKeyTimeSigPageModel::useParensRepeatCourtesiesAfterCancelling() const
{
    return styleItem(StyleId::useParensRepeatCourtesiesAfterCancelling);
}

StyleItem* ClefKeyTimeSigPageModel::showCourtesiesAfterCancellingOtherJumps() const
{
    return styleItem(StyleId::showCourtesiesAfterCancellingOtherJumps);
}

StyleItem* ClefKeyTimeSigPageModel::useParensOtherJumpCourtesiesAfterCancelling() const
{
    return styleItem(StyleId::useParensOtherJumpCourtesiesAfterCancelling);
}

StyleItem* ClefKeyTimeSigPageModel::smallParens() const
{
    return styleItem(StyleId::smallParens);
}
