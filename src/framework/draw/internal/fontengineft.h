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
#ifndef MU_DRAW_FONTENGINEFT_H
#define MU_DRAW_FONTENGINEFT_H

#include <QRectF>

#include "io/path.h"

namespace mu::draw {
struct FTData;
struct FTGlyphMetrics;
class FontEngineFT
{
public:
    FontEngineFT();
    ~FontEngineFT();

    bool load(const io::path_t& path);

    QRectF bbox(char32_t ucs4, double DPI_F) const;
    double advance(char32_t ucs4, double DPI_F) const;

private:

    FTGlyphMetrics* glyphMetrics(char32_t ucs4) const;

    FTData* m_data = nullptr;
};
}

#endif // MU_DRAW_FONTENGINEFT_H
