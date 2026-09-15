/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "watermarkpagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

WatermarkPageModel::WatermarkPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::watermarkEnabled,
                                         StyleId::watermarkType,
                                         StyleId::watermarkText,
                                         StyleId::watermarkOpacity,
                                         StyleId::watermarkAngle,
                                         StyleId::watermarkImagePath,
                                         StyleId::watermarkImageScale,
                                       })
{
}

StyleItem* WatermarkPageModel::watermarkEnabled() const { return styleItem(StyleId::watermarkEnabled); }
StyleItem* WatermarkPageModel::watermarkType() const { return styleItem(StyleId::watermarkType); }
StyleItem* WatermarkPageModel::watermarkText() const { return styleItem(StyleId::watermarkText); }
StyleItem* WatermarkPageModel::watermarkOpacity() const { return styleItem(StyleId::watermarkOpacity); }
StyleItem* WatermarkPageModel::watermarkAngle() const { return styleItem(StyleId::watermarkAngle); }
StyleItem* WatermarkPageModel::watermarkImagePath() const { return styleItem(StyleId::watermarkImagePath); }
StyleItem* WatermarkPageModel::watermarkImageScale() const { return styleItem(StyleId::watermarkImageScale); }
