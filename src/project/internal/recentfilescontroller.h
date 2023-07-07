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
#ifndef MU_PROJECT_RECENTFILESCONTROLLER_H
#define MU_PROJECT_RECENTFILESCONTROLLER_H

#include "irecentfilescontroller.h"

#include <mutex>
#include <map>

#include "async/asyncable.h"
#include "async/promise.h"

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "imscmetareader.h"
#include "io/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"

namespace mu::project {
class RecentFilesController : public IRecentFilesController, public async::Asyncable
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(IMscMetaReader, mscMetaReader)
    INJECT(io::IFileSystem, fileSystem)
    INJECT(mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    void init();

    const ProjectFilesList& recentFilesList() const override;
    async::Notification recentFilesListChanged() const override;

    void prependRecentFile(const ProjectFile& file) override;
    void moveRecentFile(const io::path_t& before, const ProjectFile& after) override;
    void removeRecentFile(const io::path_t& path) override;
    void clearRecentFiles() override;

    async::Promise<QPixmap> thumbnail(const io::path_t& file) const override;

protected:
    virtual void prependPlatformRecentFile(const io::path_t& path);
    virtual void clearPlatformRecentFiles();

private:
    void loadRecentFilesList();
    void removeNonexistentFiles();
    void setRecentFilesList(const ProjectFilesList& list, bool saveAndNotify);
    void saveRecentFilesList();

    void cleanUpThumbnailCache(const ProjectFilesList& files);

    mutable bool m_dirty = true;
    mutable ProjectFilesList m_recentFilesList;
    async::Notification m_recentFilesListChanged;
    mutable bool m_isSaving = false;

    struct CachedThumbnail {
        QPixmap thumbnail;
        DateTime lastModified;
    };

    mutable std::mutex m_thumbnailCacheMutex;
    mutable std::map<io::path_t, CachedThumbnail> m_thumbnailCache;
};
}

#endif // MU_PROJECT_RECENTFILESCONTROLLER_H
