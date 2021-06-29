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
#ifndef MU_USERSCORES_IUSERSCORESCONFIGURATION_H
#define MU_USERSCORES_IUSERSCORESCONFIGURATION_H

#include <QStringList>
#include <QColor>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "userscorestypes.h"
#include "notation/inotation.h"
#include "async/channel.h"

namespace mu::userscores {
class IUserScoresConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUserScoresConfiguration)

public:
    virtual ~IUserScoresConfiguration() = default;

    virtual ValCh<io::paths> recentScorePaths() const = 0;
    virtual void setRecentScorePaths(const io::paths& recentScorePaths) = 0;

    virtual io::path myFirstScorePath() const = 0;

    virtual io::paths availableTemplatesPaths() const = 0;

    virtual io::path userTemplatesPath() const = 0;
    virtual void setUserTemplatesPath(const io::path& path) = 0;
    virtual async::Channel<io::path> userTemplatesPathChanged() const = 0;

    virtual io::path userScoresPath() const = 0;
    virtual void setUserScoresPath(const io::path& path) = 0;
    virtual async::Channel<io::path> userScoresPathChanged() const = 0;

    virtual io::path defaultSavingFilePath(const io::path& fileName) const = 0;

    virtual QColor templatePreviewBackgroundColor() const = 0;
    virtual async::Notification templatePreviewBackgroundChanged() const = 0;

    enum class PreferredScoreCreationMode {
        FromInstruments,
        FromTemplate
    };

    virtual PreferredScoreCreationMode preferredScoreCreationMode() const = 0;
    virtual void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) = 0;

    virtual bool needShowWarningAboutUnsavedScore() const = 0;
    virtual void setNeedShowWarningAboutUnsavedScore(bool value) = 0;
};
}

#endif // MU_USERSCORES_IUSERSCORESCONFIGURATION_H
