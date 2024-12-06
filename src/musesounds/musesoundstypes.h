/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "types/string.h"
#include "types/uri.h"

namespace mu::musesounds {
struct SoundLibraryInfo {
    muse::String code;
    muse::String title;
    muse::String subtitle;
    muse::UriQuery thumbnail;
    muse::UriQuery uri;
};
using SoundLibraryInfoList = std::vector<SoundLibraryInfo>;

struct SoundCatalogueInfo {
    muse::String title;

    SoundLibraryInfoList soundLibraries;
};
using SoundCatalogueInfoList = std::vector<SoundCatalogueInfo>;
}
