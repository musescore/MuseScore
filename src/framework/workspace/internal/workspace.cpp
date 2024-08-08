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

#include "multiinstances/resourcelockguard.h"

#include "workspacefile.h"
#include "workspaceerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::workspace;

Workspace::Workspace(const io::path_t& filePath, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_file(filePath)
{
    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName){
        if (resourceName == fileResourceName()) {
            reload();
        }
    });
}

std::string Workspace::name() const
{
    return io::completeBasename(m_file.filePath()).toStdString();
}

std::string Workspace::title() const
{
    return name();
}

bool Workspace::isManaged(const DataKey& key) const
{
    return m_file.meta(key).toBool();
}

void Workspace::setIsManaged(const DataKey& key, bool val)
{
    m_file.setMeta(key, Val(val));
}

RetVal<QByteArray> Workspace::rawData(const DataKey& key) const
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_file.isLoaded()) {
        return RetVal<QByteArray>(make_ret(Err::NotLoaded));
    }

    RetVal<QByteArray> rv;
    rv.ret = make_ret(Ret::Code::Ok);
    rv.val = m_file.data(key);
    return rv;
}

Ret Workspace::setRawData(const DataKey& key, const QByteArray& data)
{
    m_file.setData(key, data);
    return make_ret(Ret::Code::Ok);
}

async::Notification Workspace::reloadNotification()
{
    return m_reloadNotification;
}

bool Workspace::isLoaded() const
{
    return m_file.isLoaded();
}

io::path_t Workspace::filePath() const
{
    return m_file.filePath();
}

Ret Workspace::load()
{
    mi::ReadResourceLockGuard resource_guard(multiInstancesProvider.get(), fileResourceName());
    return m_file.load();
}

Ret Workspace::save()
{
    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider.get(), fileResourceName());
    m_file.setMeta("app_version", Val(application()->version().toStdString()));
    return m_file.save();
}

void Workspace::reload()
{
    load();
    m_reloadNotification.notify();
}

std::string Workspace::fileResourceName() const
{
    return filePath().toStdString();
}
