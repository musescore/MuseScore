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
#include "recentprojectsprovider.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;

void RecentProjectsProvider::init()
{
    m_dirty = true;

    configuration()->recentProjectPathsChanged().onReceive(this, [this](const io::paths&) {
        m_dirty = true;
        m_recentListChanged.notify();
    });
}

ProjectMetaList RecentProjectsProvider::recentProjectList() const
{
    if (m_dirty) {
        io::paths paths = configuration()->recentProjectPaths();
        m_recentList.clear();
        for (const io::path& path : paths) {
            RetVal<ProjectMeta> meta = mscMetaReader()->readMeta(path);
            if (!meta.ret) {
                LOGE() << "failed read meta, path: " << path;
                continue;
            }
            m_recentList.push_back(std::move(meta.val));
        }
        m_dirty = false;
    }

    return m_recentList;
}

mu::async::Notification RecentProjectsProvider::recentProjectListChanged() const
{
    return m_recentListChanged;
}
