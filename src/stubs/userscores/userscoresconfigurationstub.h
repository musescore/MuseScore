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
#ifndef MU_USERSCORES_USERSCORESCONFIGURATIONSTUB_H
#define MU_USERSCORES_USERSCORESCONFIGURATIONSTUB_H

#include "userscores/iuserscoresconfiguration.h"

namespace mu::userscores {
class UserScoresConfigurationStub : public IUserScoresConfiguration
{
public:
    ValCh<QStringList> recentScorePaths() const override;
    void setRecentScorePaths(const QStringList& recentScorePaths) override;

    io::paths templatesDirPaths() const override;
    io::path scoresPath() const override;
    io::path defaultSavingFilePath(const std::string& fileName) const override;

    QColor templatePreviewBackgroundColor() const override;
    async::Channel<QColor> templatePreviewBackgroundChanged() const override;
};
}

#endif // MU_USERSCORES_USERSCORESCONFIGURATIONSTUB_H
