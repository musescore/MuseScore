/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QString>
#include <QUrl>

#include "io/path.h"

#include "cloud/cloudtypes.h"

namespace mu::project {
struct OpenParams {
    OpenParams() {}

    muse::io::path_t stylePath;
    bool disablePlayback = false;
    bool forceMode = false;
    bool forcePageMode = false;
    bool unrollRepeats = false;
};

enum class SaveMode
{
    Save,
    SaveAs,
    SaveCopy,
    SaveSelection,
    AutoSave,
    SavePage,
};

struct CloudProjectInfo {
    QUrl sourceUrl;
    int revisionId = 0;
    QString name;

    muse::cloud::Visibility visibility = muse::cloud::Visibility::Private;

    bool isValid() const
    {
        return !sourceUrl.isEmpty();
    }
};

struct CloudAudioInfo {
    QString name;
    QUrl url;
    muse::cloud::Visibility visibility = muse::cloud::Visibility::Private;
    bool replaceExisting = false;

    bool isValid() const
    {
        return !name.isEmpty();
    }
};

enum class GenerateAudioTimePeriodType {
    Never = 0,
    Always,
    AfterCertainNumberOfSaves
};
}
