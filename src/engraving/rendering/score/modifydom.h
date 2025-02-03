/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_MODIFYDOM_DEV_H
#define MU_ENGRAVING_MODIFYDOM_DEV_H

#include "layoutcontext.h"

namespace mu::engraving {
class Measure;
}

//! NOTE Here collected functions of DOM modification, witch called on layout stage.
//! They probably need to be called either before the layout or directly during the DOM modification,
//! but not during the layout.
namespace mu::engraving::rendering::score {
class ModifyDom
{
public:

    static void setCrossMeasure(const Measure* measure, LayoutContext& ctx);
    static void connectTremolo(Measure* m);
    static void cmdUpdateNotes(const Measure* measure, const DomAccessor& dom);
    static void createStems(const Measure* measure, LayoutContext& ctx);
    static void setTrackForChordGraceNotes(Measure* measure, const DomAccessor& dom);
    static void sortMeasureSegments(Measure* measure, LayoutContext& ctx);
private:
    static void removeAndAddBeginSegments(Measure* measure);
};
}

#endif // MU_ENGRAVING_MODIFYDOM_DEV_H
