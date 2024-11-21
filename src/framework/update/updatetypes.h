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

    std::string imageUrl;           // it can be base64 data, like "data:image/png;base64,iVBORw0KGgoA......"
    std::string notes;
    PrevReleasesNotesList previousReleasesNotes;

    ValMap additionInfo;

    std::string actionTitle;        // title of action button
    std::string cancelTitle;        // title of cancel button
    ValList actions;                // open app or web page url, try in order

    bool isValid() const
    {
        return !version.empty();
    }
};

static constexpr int AUTO_CHECK_UPDATE_INTERVAL = 1000;

static ValList releasesNotesToValList(const PrevReleasesNotesList& list)
{
    ValList valList;
    for (const PrevReleaseNotes& release : list) {
        valList.emplace_back(Val(ValMap {
                { "version", Val(release.version) },
                { "notes", Val(release.notes) }
            }));
    }

    return valList;
}

static PrevReleasesNotesList releasesNotesFromValList(const ValList& list)
{
    PrevReleasesNotesList notes;
    for (const Val& val : list) {
        ValMap releaseMap = val.toMap();
        notes.emplace_back(releaseMap.at("version").toString(), releaseMap.at("notes").toString());
    }

    return notes;
}

static inline ValMap releaseInfoToValMap(const ReleaseInfo& info)
{
    return {
        { "version", Val(info.version) },
        { "fileName", Val(info.fileName) },
        { "fileUrl", Val(info.fileUrl) },
        { "notes", Val(info.notes) },
        { "previousReleasesNotes", Val(releasesNotesToValList(info.previousReleasesNotes)) },
        { "additionalInfo", Val(info.additionInfo) },
        { "imageUrl", Val(info.imageUrl) },
        { "actionTitle", Val(info.actionTitle) },
        { "cancelTitle", Val(info.cancelTitle) },
        { "actions", Val(info.actions) },
    };
}

static inline ReleaseInfo releaseInfoFromValMap(const ValMap& map)
{
    ReleaseInfo info;
    info.version = map.at("version").toString();
    info.fileName = map.at("fileName").toString();
    info.fileUrl = map.at("fileUrl").toString();
    info.notes = map.at("notes").toString();
    info.previousReleasesNotes = releasesNotesFromValList(map.at("previousReleasesNotes").toList());
    info.additionInfo = map.at("additionalInfo").toMap();
    info.imageUrl = map.at("imageUrl").toString();
    info.actionTitle = map.at("actionTitle").toString();
    info.cancelTitle = map.at("cancelTitle").toString();
    info.actions = map.at("actions").toList();

    return info;
}
}

#endif // MUSE_UPDATE_UPDATETYPES_H
