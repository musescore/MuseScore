/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "accidentalspagemodel.h"

using namespace mu::notation;

AccidentalsPageModel::AccidentalsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::bracketedAccidentalPadding,

    StyleId::accidentalOrderFollowsNoteDisplacement,
    StyleId::alignAccidentalOctavesAcrossSubChords,
    StyleId::keepAccidentalSecondsTogether,
    StyleId::alignOffsetOctaveAccidentals
})
{
}

StyleItem* AccidentalsPageModel::bracketedAccidentalPadding() const
{
    return styleItem(StyleId::bracketedAccidentalPadding);
}

StyleItem* AccidentalsPageModel::accidFollowNoteOffset() const
{
    return styleItem(StyleId::accidentalOrderFollowsNoteDisplacement);
}

StyleItem* AccidentalsPageModel::alignAccidentalOctavesAcrossSubChords() const
{
    return styleItem(StyleId::alignAccidentalOctavesAcrossSubChords);
}

StyleItem* AccidentalsPageModel::keepAccidentalSecondsTogether() const
{
    return styleItem(StyleId::keepAccidentalSecondsTogether);
}

StyleItem* AccidentalsPageModel::alignOffsetOctaveAccidentals() const
{
    return styleItem(StyleId::alignOffsetOctaveAccidentals);
}
