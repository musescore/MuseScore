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
#include "accidentalgrouppagemodel.h"

using namespace mu::notation;

AccidentalGroupPageModel::AccidentalGroupPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::accidentalOrderFollowsNoteDisplacement,
                                         StyleId::alignAccidentalOctavesAcrossSubChords,
                                         StyleId::keepAccidentalSecondsTogether,
                                         StyleId::alignOffsetOctaveAccidentals })
{
}

StyleItem* AccidentalGroupPageModel::accidFollowNoteOffset() const
{
    return styleItem(StyleId::accidentalOrderFollowsNoteDisplacement);
}

StyleItem* AccidentalGroupPageModel::alignAccidentalOctavesAcrossSubChords() const
{
    return styleItem(StyleId::alignAccidentalOctavesAcrossSubChords);
}

StyleItem* AccidentalGroupPageModel::keepAccidentalSecondsTogether() const
{
    return styleItem(StyleId::keepAccidentalSecondsTogether);
}

StyleItem* AccidentalGroupPageModel::alignOffsetOctaveAccidentals() const
{
    return styleItem(StyleId::alignOffsetOctaveAccidentals);
}
