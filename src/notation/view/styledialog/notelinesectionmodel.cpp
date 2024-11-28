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

#include "notelinesectionmodel.h"

using namespace mu::notation;

NoteLineSectionModel::NoteLineSectionModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::noteLineStyle,
    StyleId::noteLineDashLineLen,
    StyleId::noteLineDashGapLen,
    StyleId::noteLineWidth,
})
{
}

StyleItem* NoteLineSectionModel::noteLineStyle() const { return styleItem(StyleId::noteLineStyle); }
StyleItem* NoteLineSectionModel::noteLineStyleDashSize() const { return styleItem(StyleId::noteLineDashLineLen); }
StyleItem* NoteLineSectionModel::noteLineStyleGapSize()  const { return styleItem(StyleId::noteLineDashGapLen); }
StyleItem* NoteLineSectionModel::noteLineWidth()  const { return styleItem(StyleId::noteLineWidth); }
