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

#include "filewatcher.h"

#ifndef NO_QT_SUPPORT
#include <QFileSystemWatcher>
#endif

namespace muse::io {
#ifndef NO_QT_SUPPORT

FileWatcher::FileWatcher()
    : m_watcher(std::make_unique<QFileSystemWatcher>())
{
    QObject::connect(m_watcher.get(), &QFileSystemWatcher::fileChanged,
                     [this](const QString& path) {
        m_channel.send(path.toStdString());
    });
}

FileWatcher::~FileWatcher() = default;

void FileWatcher::startWatching(const std::string& path)
{
    m_watcher->addPath(QString::fromStdString(path));
}

void FileWatcher::stopWatching(const std::string& path)
{
    if (path.empty()) {
        const auto watchedPaths = m_watcher->files();
        if (!watchedPaths.isEmpty()) {
            m_watcher->removePaths(watchedPaths);
        }
        return;
    }
    m_watcher->removePath(QString::fromStdString(path));
}

#else

FileWatcher::FileWatcher() = default;
FileWatcher::~FileWatcher() = default;

void FileWatcher::startWatching(const std::string&) {}
void FileWatcher::stopWatching(const std::string&) {}

#endif

muse::async::Channel<std::string> FileWatcher::fileChanged() const
{
    return m_channel;
}
}
