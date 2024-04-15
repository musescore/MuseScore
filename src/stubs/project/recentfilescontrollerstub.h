/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_PROJECT_RECENTFILESCONTROLLERSTUB_H
#define MU_PROJECT_RECENTFILESCONTROLLERSTUB_H

#include "project/irecentfilescontroller.h"

namespace mu::project {
class RecentFilesControllerStub : public IRecentFilesController
{
public:
    RecentFilesControllerStub() = default;

    const RecentFilesList& recentFilesList() const override;
    muse::async::Notification recentFilesListChanged() const override;

    void prependRecentFile(const RecentFile& file) override;
    void moveRecentFile(const muse::io::path_t& before, const RecentFile& after) override;
    void clearRecentFiles() override;

    muse::async::Promise<QPixmap> thumbnail(const RecentFile& file) const override;
};
}

#endif // MU_PROJECT_RECENTFILESCONTROLLERSTUB_H
