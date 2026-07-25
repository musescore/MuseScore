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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include <QString>

#include "io/path.h"

#include "projectfile.h"

namespace mu::project {
struct RecentFile {
    muse::io::path_t path;
    QString displayNameOverride = {};

    RecentFile() = default;

    RecentFile(const muse::io::path_t& path, const QString& displayNameOverride = {})
        : path(path), displayNameOverride(displayNameOverride) {}

    static RecentFile fromProjectFile(const ProjectFile& projectFile)
    {
        return RecentFile(projectFile.path(), projectFile.displayNameOverride);
    }

    ProjectFile toProjectFile() const
    {
        return ProjectFile(path, displayNameOverride);
    }

    bool isValid() const
    {
        return !path.empty();
    }

    QString displayName(bool includingExtension) const
    {
        if (!displayNameOverride.isEmpty()) {
            return displayNameOverride;
        }

        return muse::io::filename(path, includingExtension).toQString();
    }

    bool operator ==(const RecentFile& other) const
    {
        return path == other.path
               && displayNameOverride == other.displayNameOverride;
    }

    bool operator !=(const RecentFile& other) const
    {
        return !(*this == other);
    }
};

using RecentFilesList = std::vector<RecentFile>;
}
