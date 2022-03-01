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
#ifndef MU_PROJECT_IPROJECTVIEWSETTINGS_H
#define MU_PROJECT_IPROJECTVIEWSETTINGS_H

#include <memory>

#include "retval.h"
#include "notation/notationtypes.h"

namespace mu::project {
class IProjectViewSettings
{
public:
    virtual ~IProjectViewSettings() = default;

    virtual notation::ViewMode notationViewMode() const = 0;
    virtual void setNotationViewMode(const notation::ViewMode& mode) = 0;

    virtual mu::ValNt<bool> needSave() const = 0;
};

using IProjectViewSettingsPtr = std::shared_ptr<IProjectViewSettings>;
}

#endif // MU_PROJECT_IPROJECTVIEWSETTINGS_H
