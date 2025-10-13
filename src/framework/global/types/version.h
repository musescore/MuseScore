/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include <string>

#include "types/string.h"

namespace muse {
class Version
{
public:
    Version() = default;
    Version(int major, int minor = 0, int patch = 0, const String& suffix = String(), int suffixVersion = 0);
    Version(const String& versionStr);
    Version(const std::string& versionStr);

    int major() const;
    int minor() const;
    int patch() const;

    const String& suffix() const;
    int suffixVersion() const;
    void setSuffix(const String& suffix);

    bool isNull() const;

    bool preRelease() const;

    String toString() const;
    std::string toStdString() const;

    bool operator <(const Version& other) const;
    bool operator ==(const Version& other) const;
    bool operator <=(const Version& other) const;

private:
    int m_major = 0;
    int m_minor = 0;
    int m_patch = 0;
    String m_suffix;
    int m_suffixVersion = 0;
};
}
