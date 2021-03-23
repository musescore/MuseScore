//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

    void init();

    ValCh<io::paths> recentScorePaths() const override;
    void setRecentScorePaths(const io::paths& recentScorePaths) override;

    io::path myFirstScorePath() const override;

    io::paths availableTemplatesPaths() const override;

    ValCh<io::path> templatesPath() const override;
    void setTemplatesPath(const io::path& path) override;

    ValCh<io::path> scoresPath() const override;
    void setScoresPath(const io::path& path) override;

    io::path defaultSavingFilePath(const io::path& fileName) const override;

    QColor templatePreviewBackgroundColor() const override;
    async::Notification templatePreviewBackgroundChanged() const override;

    PreferredScoreCreationMode preferredScoreCreationMode() const override;
    void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) override;

private:
    io::path mainTemplatesDirPath() const;

    io::paths actualRecentScorePaths() const;
    io::paths parsePaths(const mu::Val& value) const;

    io::path userTemplatesPath() const;
    io::path defaultTemplatesPath() const;

    async::Channel<io::paths> m_recentScorePathsChanged;
    async::Channel<io::path> m_templatesPathChanged;
    async::Channel<io::path> m_scoresPathChanged;
};
}

#endif // MU_USERSCORES_USERSCORECONFIGURATION_H
