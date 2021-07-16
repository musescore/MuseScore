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

#include "style.h"

namespace mu::engraving {
class DefaultStyle
{
public:

    static DefaultStyle* instance();

    void init(const QString& defaultSyleFilePath, const QString& partStyleFilePath);

    static inline const Ms::MStyle& baseStyle() { return instance()->m_baseStyle; }
    static inline const Ms::MStyle& defaultStyle() { return instance()->m_defaultStyle; }
    static inline const Ms::MStyle* defaultStyleForParts() { return instance()->m_defaultStyleForParts; }

    static const Ms::MStyle* resolveStyleDefaults(const int defaultsVersion);

private:
    DefaultStyle() = default;

    bool readDefaultStyle(QString file);
    bool readPartStyle(QString filePath);

    Ms::MStyle m_baseStyle; // buildin initial style
    Ms::MStyle m_defaultStyle; // buildin modified by preferences
    Ms::MStyle* m_defaultStyleForParts = nullptr;
};
}

#endif // MU_ENGRAVING_DEFAULTSTYLE_H
