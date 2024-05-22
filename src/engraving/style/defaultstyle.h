/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_DEFAULTSTYLE_H
#define MU_ENGRAVING_DEFAULTSTYLE_H

#include "global/io/path.h"

#include "style.h"

namespace mu::engraving {
class DefaultStyle
{
public:
    static DefaultStyle* instance();

    void init(const muse::io::path_t& defaultStyleFilePath, const muse::io::path_t& partStyleFilePath, const SizeF& defaultPageSize);

    static const MStyle& baseStyle();

    static const MStyle& defaultStyle();
    static const MStyle* defaultStyleForParts();

    static const MStyle& resolveStyleDefaults(const int defaultsVersion);

private:
    DefaultStyle() = default;

    static bool doLoadStyle(MStyle* style, const muse::io::path_t& filePath);

    MStyle m_baseStyle; // builtin initial style
    MStyle m_defaultStyle; // builtin modified by preferences
    MStyle* m_defaultStyleForParts = nullptr;
};
}

#endif // MU_ENGRAVING_DEFAULTSTYLE_H
