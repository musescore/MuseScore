/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "toursconfigurationstub.h"

using namespace muse::tours;

muse::String ToursConfigurationStub::lastShownTourIdForEvent(const String&) const
{
    return u"";
}

void ToursConfigurationStub::setLastShownTourIdForEvent(const String&, const String&)
{
}

muse::io::path_t ToursConfigurationStub::toursFilePath() const
{
    return "";
}
