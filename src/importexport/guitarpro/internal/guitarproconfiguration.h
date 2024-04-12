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
#ifndef MU_IMPORTEXPORT_GUITARPROCONFIGURATION_H
#define MU_IMPORTEXPORT_GUITARPROCONFIGURATION_H

#include "../iguitarproconfiguration.h"

namespace mu::iex::guitarpro {
class GuitarProConfiguration : public IGuitarProConfiguration
{
public:
    bool linkedTabStaffCreated() const override;
    void setLinkedTabStaffCreated(std::optional<bool> created) override;

    bool experimental() const override;
    void setExperimental(std::optional<bool> experimental) override;

private:
    std::optional<bool> m_linkedTabStaffCreated;
    std::optional<bool> m_experimental;
};
}

#endif // MU_IMPORTEXPORT_GUITARPROCONFIGURATION_H
