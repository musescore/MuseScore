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
#ifndef MU_USERSCORES_IUSERSCORESCONFIGURATION_H
#define MU_USERSCORES_IUSERSCORESCONFIGURATION_H

#include <QStringList>
#include <QColor>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "userscorestypes.h"

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

    virtual ValCh<io::path> templatesPath() const = 0;
    virtual void setTemplatesPath(const io::path& path) = 0;

    virtual ValCh<io::path> scoresPath() const = 0;
    virtual void setScoresPath(const io::path& path) = 0;

    virtual io::path defaultSavingFilePath(const io::path& fileName) const = 0;

    virtual QColor templatePreviewBackgroundColor() const = 0;
    virtual async::Notification templatePreviewBackgroundChanged() const = 0;

    enum class PreferredScoreCreationMode {
        FromInstruments,
        FromTemplate
    };

    virtual PreferredScoreCreationMode preferredScoreCreationMode() const = 0;
    virtual void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) = 0;
};
}

#endif // MU_USERSCORES_IUSERSCORESCONFIGURATION_H
