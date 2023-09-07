/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_BIND_H
#define MU_ENGRAVING_BIND_H

#include "global/async/channel.h"

namespace mu::engraving {
template<typename T>
class Bind
{
public:
    Bind() = default;
    Bind(const T& v)
        : m_val(v) {}

    void operator=(const T& v) { setVal(v); }

    bool operator==(const T& v) const { return m_val == v; }
    bool operator!=(const T& v) const { return m_val != v; }

    operator T() const {
        return m_val;
    }

    const T& val() const { return m_val; }
    void setVal(const T& v)
    {
        if (m_val != v) {
            m_val = v;
            m_ch.send(v);
        }
    }

    async::Channel<T> changed() const { return m_ch; }

private:

    T m_val = T();
    async::Channel<T> m_ch;
};
}

#endif // MU_ENGRAVING_BIND_H
