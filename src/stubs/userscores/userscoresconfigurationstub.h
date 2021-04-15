/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
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
