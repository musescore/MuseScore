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
#ifndef MU_DIAGNOSTICS_DRAWDATACONVERTER_H
#define MU_DIAGNOSTICS_DRAWDATACONVERTER_H

#include "global/types/ret.h"
#include "global/io/path.h"
#include "draw/types/drawdata.h"
#include "draw/types/pixmap.h"

namespace mu::diagnostics {
class DrawDataConverter
{
public:
    DrawDataConverter() = default;

    Ret drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile) const;
    draw::Pixmap drawDataToPixmap(const draw::DrawDataPtr& data) const;
    void drawOnPixmap(draw::Pixmap& px, const draw::DrawDataPtr& data, const draw::Color& overlay = draw::Color()) const;
};
}

#endif // MU_DIAGNOSTICS_DRAWDATACONVERTER_H
