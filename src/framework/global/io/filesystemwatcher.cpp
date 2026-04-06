/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "filesystemwatcher.h"

#ifndef NO_QT_SUPPORT
#include <QFileSystemWatcher>
#endif

namespace muse::io {
#ifndef NO_QT_SUPPORT

FileSystemWatcher::FileSystemWatcher()
    : m_watcher(std::make_unique<QFileSystemWatcher>())
{
    QObject::connect(m_watcher.get(), &QFileSystemWatcher::fileChanged,
                     [this](const QString& path) {
        m_fileChannel.send(path.toStdString());
    });

    QObject::connect(m_watcher.get(), &QFileSystemWatcher::directoryChanged,
                     [this](const QString& path) {
        m_directoryChannel.send(path.toStdString());
    });
}

FileSystemWatcher::~FileSystemWatcher() = default;

void FileSystemWatcher::startWatching(const std::string& path)
{
    m_watcher->addPath(QString::fromStdString(path));
}

void FileSystemWatcher::stopWatching(const std::string& path)
{
    if (path.empty()) {
        const auto watchedPaths = m_watcher->files();
        if (!watchedPaths.isEmpty()) {
            m_watcher->removePaths(watchedPaths);
        }
        const auto watchedDirs = m_watcher->directories();
        if (!watchedDirs.isEmpty()) {
            m_watcher->removePaths(watchedDirs);
        }
        return;
    }
    m_watcher->removePath(QString::fromStdString(path));
}

#else

FileSystemWatcher::FileSystemWatcher() = default;
FileSystemWatcher::~FileSystemWatcher() = default;

void FileSystemWatcher::startWatching(const std::string&) {}
void FileSystemWatcher::stopWatching(const std::string&) {}

#endif

muse::async::Channel<std::string> FileSystemWatcher::fileChanged() const
{
    return m_fileChannel;
}

muse::async::Channel<std::string> FileSystemWatcher::directoryChanged() const
{
    return m_directoryChannel;
}
}
