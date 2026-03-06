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
#pragma once

#include "global/types/string.h"

namespace mu::engraving {
class StaffName
{
public:
    StaffName() = default;
    StaffName(const muse::String& longName, const muse::String& shortName)
        : m_longName(longName), m_shortName(shortName) {}

    bool operator==(const StaffName& i) const { return m_longName == i.m_longName && m_shortName == i.m_shortName; }

    const muse::String& longName() const { return m_longName; }
    const muse::String& shortName() const { return m_shortName; }
    void setLongName(const muse::String& s) { m_longName = s; }
    void setShortName(const muse::String& s) { m_shortName = s; }

    bool empty() const { return m_longName.empty() && m_shortName.empty(); }

private:
    muse::String m_longName;
    muse::String m_shortName;
};
}
