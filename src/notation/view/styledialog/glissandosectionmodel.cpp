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

#include "glissandosectionmodel.h"

using namespace mu::notation;

GlissandoSectionModel::GlissandoSectionModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::glissandoLineStyle,
    StyleId::glissandoDashLineLen,
    StyleId::glissandoDashGapLen,
    StyleId::glissandoLineWidth,
    StyleId::glissandoShowText,
    StyleId::glissandoText,
    StyleId::glissandoType
})
{
}

StyleItem* GlissandoSectionModel::glissandoLineStyle() const { return styleItem(StyleId::glissandoLineStyle); }
StyleItem* GlissandoSectionModel::glissandoLineStyleDashSize() const { return styleItem(StyleId::glissandoDashLineLen); }
StyleItem* GlissandoSectionModel::glissandoLineStyleGapSize()  const { return styleItem(StyleId::glissandoDashGapLen); }
StyleItem* GlissandoSectionModel::glissandoLineWidth()  const { return styleItem(StyleId::glissandoLineWidth); }
StyleItem* GlissandoSectionModel::glissandoShowText()  const { return styleItem(StyleId::glissandoShowText); }
StyleItem* GlissandoSectionModel::glissandoText()  const { return styleItem(StyleId::glissandoText); }
StyleItem* GlissandoSectionModel::glissandoLineType()  const { return styleItem(StyleId::glissandoType); }
