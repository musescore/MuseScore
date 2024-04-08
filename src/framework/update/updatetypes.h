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
#ifndef MUSE_UPDATE_UPDATETYPES_H
#define MUSE_UPDATE_UPDATETYPES_H

#include <string>

#include "types/val.h"

namespace muse::update {
struct PrevReleaseNotes {
    std::string version;
    std::string notes;

    PrevReleaseNotes() = default;
    PrevReleaseNotes(const std::string& version, const std::string& notes)
        : version(version), notes(notes) {}

    bool operator ==(const PrevReleaseNotes& other) const
    {
        return version == other.version && notes == other.notes;
    }
};
using PrevReleasesNotesList = std::vector<PrevReleaseNotes>;

struct ReleaseInfo {
    std::string version;
    std::string fileName;
    std::string fileUrl;

    std::string notes;
    PrevReleasesNotesList previousReleasesNotes;

    ValMap additionInfo;

    bool isValid() const
    {
        return !version.empty();
    }
};
}

#endif // MUSE_UPDATE_UPDATETYPES_H
