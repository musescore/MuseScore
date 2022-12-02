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
#ifndef MU_ENGRAVING_DEFAULTSTYLE_H
#define MU_ENGRAVING_DEFAULTSTYLE_H

#include "io/path.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

#include "style.h"

namespace mu::engraving {
class DefaultStyle
{
    INJECT(engraving, IEngravingConfiguration, engravingConfiguration)

public:
    static DefaultStyle* instance();

    void init(const io::path_t& defaultStyleFilePath, const io::path_t& partStyleFilePath);

    static const MStyle& baseStyle();

    static const MStyle& defaultStyle();
    static const MStyle* defaultStyleForParts();

    static const MStyle& resolveStyleDefaults(const int defaultsVersion);

private:
    DefaultStyle() = default;

    static bool doLoadStyle(MStyle* style, const io::path_t& filePath);

    MStyle m_baseStyle; // builtin initial style
    MStyle m_defaultStyle; // builtin modified by preferences
    MStyle* m_defaultStyleForParts = nullptr;
};
}

#endif // MU_ENGRAVING_DEFAULTSTYLE_H
