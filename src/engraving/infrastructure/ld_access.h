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
#ifndef MU_ENGRAVING_LD_ACCESS_DEV_H
#define MU_ENGRAVING_LD_ACCESS_DEV_H

#include <optional>

#include "log.h"

namespace mu::engraving {
//! NOTE Enum for detect layoutdata bad access errors
enum class LD_ACCESS {
    CHECK = 0,          // should be correct; assert if still not
    BAD,                // known to be bad; don’t assert, we’ll fix it later
    MAYBE_NOTINITED,    // in this case it’s okay if we access it before it’s been inited
    PASS                // for debug
};

#ifdef MUE_ENABLE_ENGRAVING_LD_ACCESS

//! TODO Using this macro, we can collect and output debugging information to show the dependency tree
#define LD_CONDITION(val) \
    if (!val) { \
        LOGE_T("LD_ACCESS")() << "BAD ACCESS to: " << #val; \
    } \

#else
#define LD_CONDITION(val) (void)val;
#endif

// mark as independent
#define LD_INDEPENDENT

template<typename T>
class ld_field
{
public:

    ld_field(const char* name, T def = T())
        : m_name(name), m_def(def) {}

    void reset() { m_val.reset(); }

    bool has_value() const { return m_val.has_value(); }

    const T& value(LD_ACCESS mode = LD_ACCESS::CHECK) const
    {
        if (!m_val.has_value()) {
#ifdef MUE_ENABLE_ENGRAVING_LD_ACCESS
            if (mode == LD_ACCESS::CHECK) {
                LOGE_T("LD_ACCESS")() << "BAD ACCESS to: " << m_name;
            }
#else
            UNUSED(mode);
#endif
            return m_def;
        }
        return m_val.value();
    }

    const T& operator()() const
    {
        return value();
    }

    T& mut_value()
    {
        if (!m_val.has_value()) {
            m_val = std::make_optional<T>(m_def);
        }
        return m_val.value();
    }

    void set_value(const T& v)
    {
        m_val = std::make_optional<T>(v);
    }

private:
    const char* m_name = nullptr;
    T m_def;
    std::optional<T> m_val;
};
}

#endif // MU_ENGRAVING_LD_ACCESS_DEV_H
