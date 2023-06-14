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
#include "recentfilescontroller.h"

#include <QtConcurrent>

#include <QJsonArray>
#include <QJsonDocument>

#include "async/async.h"
#include "defer.h"

#include "multiinstances/resourcelockguard.h"

using namespace mu::project;
using namespace mu::async;

static const std::string RECENT_FILES_RESOURCE_NAME("RECENT_FILES");

void RecentFilesController::init()
{
    TRACEFUNC;

    m_dirty = true;

    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == RECENT_FILES_RESOURCE_NAME) {
            if (!m_isSaving) {
                m_dirty = true;

                m_recentFilesListChanged.notify();
            }
        }
    });
}

const ProjectFilesList& RecentFilesController::recentFilesList() const
{
    TRACEFUNC;

    if (m_dirty) {
        const_cast<RecentFilesController*>(this)->loadRecentFilesList();
    }

    const_cast<RecentFilesController*>(this)->removeNonexistentFiles();

    return m_recentFilesList;
}

Notification RecentFilesController::recentFilesListChanged() const
{
    return m_recentFilesListChanged;
}

void RecentFilesController::prependRecentFile(const ProjectFile& newFile)
{
    if (!newFile.isValid()) {
        return;
    }

    TRACEFUNC;

    ProjectFilesList newList;
    newList.reserve(m_recentFilesList.size() + 1);
    newList.push_back(newFile);

    for (const ProjectFile& file : m_recentFilesList) {
        if (file.path != newFile.path && fileSystem()->exists(file.path)) {
            newList.push_back(file);
        }
    }

    setRecentFilesList(newList, true);

    prependPlatformRecentFile(newFile.path);
}

void RecentFilesController::moveRecentFile(const io::path_t& before, const ProjectFile& after)
{
    bool moved = false;
    ProjectFilesList newList = m_recentFilesList;

    for (ProjectFile& file : newList) {
        if (file.path == before) {
            file = after;
            moved = true;
            break;
        }
    }

    if (moved) {
        setRecentFilesList(newList, true);
    }
}

void RecentFilesController::clearRecentFiles()
{
    setRecentFilesList({}, true);

    clearPlatformRecentFiles();
}

void RecentFilesController::prependPlatformRecentFile(const io::path_t&) {}

void RecentFilesController::clearPlatformRecentFiles() {}

void RecentFilesController::loadRecentFilesList()
{
    ProjectFilesList newList;

    DEFER {
        setRecentFilesList(newList, false);
    };

    RetVal<ByteArray> data;
    {
        mi::ReadResourceLockGuard lock_guard(multiInstancesProvider(), RECENT_FILES_RESOURCE_NAME);
        data = fileSystem()->readFile(configuration()->recentFilesJsonPath());
    }

    if (!data.ret || data.val.empty()) {
        data.val = configuration()->compatRecentFilesData();
    }

    if (data.val.empty()) {
        return;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.val.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError) {
        return;
    }

    if (!jsonDoc.isArray()) {
        return;
    }

    for (const QJsonValue val : jsonDoc.array()) {
        if (val.isString()) {
            newList.push_back(ProjectFile { io::path_t(val.toString()) });
        } else if (val.isObject()) {
            QJsonObject obj = val.toObject();
            ProjectFile file;
            file.path = obj["path"].toString();
            file.displayNameOverride = obj["displayName"].toString();
            newList.push_back(file);
        } else {
            continue;
        }
    }
}

void RecentFilesController::removeNonexistentFiles()
{
    bool removed = false;

    ProjectFilesList newList;
    newList.reserve(m_recentFilesList.size());

    for (const ProjectFile& file : m_recentFilesList) {
        if (fileSystem()->exists(file.path)) {
            newList.push_back(file);
        } else {
            removed = true;
        }
    }

    if (removed) {
        setRecentFilesList(newList, false);

        async::Async::call(nullptr, [this, newList]() {
            saveRecentFilesList();

            m_recentFilesListChanged.notify();
        });
    }
}

void RecentFilesController::setRecentFilesList(const ProjectFilesList& list, bool saveAndNotify)
{
    if (m_recentFilesList == list) {
        return;
    }

    m_recentFilesList = list;

    cleanUpThumbnailCache(list);

    if (saveAndNotify) {
        saveRecentFilesList();

        m_recentFilesListChanged.notify();
    }
}

void RecentFilesController::saveRecentFilesList()
{
    TRACEFUNC;

    m_isSaving = true;

    DEFER {
        m_isSaving = false;
    };

    QJsonArray jsonArray;
    for (const ProjectFile& file : m_recentFilesList) {
        if (!file.displayNameOverride.isEmpty()) {
            QJsonObject obj;
            obj["path"] = file.path.toQString();
            obj["displayName"] = file.displayNameOverride;
            jsonArray << obj;
        } else {
            jsonArray << file.path.toQString();
        }
    }

    QJsonDocument jsonDoc(jsonArray);
    QByteArray json = jsonDoc.toJson(QJsonDocument::Compact);

    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), RECENT_FILES_RESOURCE_NAME);
    Ret ret = fileSystem()->writeFile(configuration()->recentFilesJsonPath(), ByteArray::fromQByteArrayNoCopy(json));
    if (!ret) {
        LOGE() << "Failed to save recent files list: " << ret.toString();
    }
}

Promise<QPixmap> RecentFilesController::thumbnail(const io::path_t& file) const
{
    return Promise<QPixmap>([this, file](auto resolve, auto reject) {
        if (file.empty()) {
            return reject(int(Ret::Code::UnknownError), "Invalid file specified");
        }

        QtConcurrent::run([this, file, resolve, reject]() {
            std::lock_guard lock(m_thumbnailCacheMutex);

            DateTime lastModified = fileSystem()->lastModified(file);

            auto it = m_thumbnailCache.find(file);
            if (it != m_thumbnailCache.cend()) {
                if (lastModified == it->second.lastModified) {
                    (void)resolve(it->second.thumbnail);
                    return;
                }
            }

            RetVal<ProjectMeta> rv = mscMetaReader()->readMeta(file);
            if (!rv.ret) {
                m_thumbnailCache[file] = CachedThumbnail();
                (void)reject(rv.ret.code(), rv.ret.toString());
            } else {
                m_thumbnailCache[file] = CachedThumbnail { rv.val.thumbnail, lastModified };
                (void)resolve(rv.val.thumbnail);
            }
        });

        return Promise<QPixmap>::Result::unchecked();
    }, Promise<QPixmap>::AsynchronyType::ProvidedByBody);
}

void RecentFilesController::cleanUpThumbnailCache(const ProjectFilesList& files)
{
    QtConcurrent::run([this, files] {
        std::lock_guard lock(m_thumbnailCacheMutex);

        if (files.empty()) {
            m_thumbnailCache.clear();
        } else {
            std::map<io::path_t, CachedThumbnail> cleanedCache;

            for (const ProjectFile& file : files) {
                auto it = m_thumbnailCache.find(file.path);
                if (it != m_thumbnailCache.cend()) {
                    cleanedCache[file.path] = it->second;
                }
            }

            m_thumbnailCache = cleanedCache;
        }
    });
}
