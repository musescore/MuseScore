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
#ifndef MU_USERSCORES_USERSCORECONFIGURATION_H
#define MU_USERSCORES_USERSCORECONFIGURATION_H

#include "iuserscoresconfiguration.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "extensions/iextensionsconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "global/val.h"
#include "system/ifilesystem.h"

namespace mu::userscores {
class UserScoresConfiguration : public IUserScoresConfiguration
{
    INJECT(userscores, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(userscores, extensions::IExtensionsConfiguration, extensionsConfiguration)
    INJECT(userscores, notation::INotationConfiguration, notationConfiguration)
    INJECT(userscores, system::IFileSystem, fileSystem)

public:
    static const QString DEFAULT_FILE_SUFFIX;
    static const QString DEFAULT_EXPORT_SUFFIX;

    void init();

    ValCh<io::paths> recentScorePaths() const override;
    void setRecentScorePaths(const io::paths& recentScorePaths) override;

    io::path myFirstScorePath() const override;

    io::paths availableTemplatesPaths() const override;

    io::path userTemplatesPath() const override;
    void setUserTemplatesPath(const io::path& path) override;
    async::Channel<io::path> userTemplatesPathChanged() const override;

    io::path userScoresPath() const override;
    void setUserScoresPath(const io::path& path) override;
    async::Channel<io::path> userScoresPathChanged() const override;

    io::path defaultSavingFilePath(const io::path& fileName) const override;

    QColor templatePreviewBackgroundColor() const override;
    async::Notification templatePreviewBackgroundChanged() const override;

    PreferredScoreCreationMode preferredScoreCreationMode() const override;
    void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) override;

    bool needShowWarningAboutUnsavedScore() const override;
    void setNeedShowWarningAboutUnsavedScore(bool value) override;

private:

    io::paths actualRecentScorePaths() const;
    io::paths parsePaths(const mu::Val& value) const;

    io::path appTemplatesPath() const;

    async::Channel<io::paths> m_recentScorePathsChanged;
    async::Channel<io::path> m_userTemplatesPathChanged;
    async::Channel<io::path> m_userScoresPathChanged;
};
}

#endif // MU_USERSCORES_USERSCORECONFIGURATION_H
