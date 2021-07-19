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
#include "workspace.h"

#include "global/version.h"
#include "multiinstances/resourcelockguard.h"

#include "workspacefile.h"
#include "workspaceerrors.h"

#include "log.h"

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

Workspace::Workspace(const io::path& filePath)
    : m_file(filePath)
{
}

std::string Workspace::name() const
{
    return io::basename(m_file.filePath()).toStdString();
}

std::string Workspace::title() const
{
    return name();
}

bool Workspace::isManaged(const DataKey& key) const
{
    return m_file.meta(key_to_string(key)).toBool();
}

void Workspace::setIsManaged(const DataKey& key, bool val)
{
    m_file.setMeta(key_to_string(key), Val(val));
}

RetVal<QByteArray> Workspace::rawData(const DataKey& key) const
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_file.isLoaded()) {
        return RetVal<QByteArray>(make_ret(Err::NotLoaded));
    }

    QByteArray data = m_file.data(key_to_string(key));
    if (data.isEmpty()) {
        return RetVal<QByteArray>(make_ret(Err::NoData));
    }

    RetVal<QByteArray> rv;
    rv.ret = make_ret(Ret::Code::Ok);
    rv.val = m_file.data(key_to_string(key));
    return rv;
}

Ret Workspace::setRawData(const DataKey& key, const QByteArray& data)
{
    m_file.setData(key_to_string(key), data);
    return make_ret(Ret::Code::Ok);
}

bool Workspace::isLoaded() const
{
    return m_file.isLoaded();
}

io::path Workspace::filePath() const
{
    return m_file.filePath();
}

Ret Workspace::load()
{
    mi::ResourceLockGuard resource_guard(multiInstancesProvider(), "WORKSPACE_FILE");
    return m_file.load();
}

Ret Workspace::save()
{
    mi::ResourceLockGuard resource_guard(multiInstancesProvider(), "WORKSPACE_FILE");
    m_file.setMeta("app_version", Val(Version::version()));
    return m_file.save();
}
