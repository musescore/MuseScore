/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

namespace mu::engraving {
class StaffType;
}

namespace mu::iex::mnxio {
//---------------------------------------------------------
//   MnxStaffConfigState
//   The staff settings an MNX staff config describes.
//
//   MNX defines a staff config as a complete description of a staff: a field it omits takes
//   its default rather than keeping the previous value. So a state is compared and replaced
//   as a whole, never merged. To support a new staff config field, add a member with its
//   MNX default here and handle it in each of the four functions below.
//---------------------------------------------------------

struct MnxStaffConfigState {
    int lines = 5;

    static MnxStaffConfigState fromMnx(const mnx::StaffConfig& config);
    static MnxStaffConfigState fromStaffType(const engraving::StaffType& staffType);

    /// Applies the state to a staff type, leaving every setting MNX does not describe as it is.
    void applyTo(engraving::StaffType& staffType) const;
    /// Writes the fields that differ from their MNX defaults.
    void writeTo(mnx::StaffConfig& config) const;

    bool operator==(const MnxStaffConfigState&) const = default;
};
} // namespace mu::iex::mnxio
