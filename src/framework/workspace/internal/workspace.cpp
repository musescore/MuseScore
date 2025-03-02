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
    : Injectable(iocCtx)
{
    m_file = std::make_shared<WorkspaceFile>(filePath);

    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName){
        if (resourceName == fileResourceName()) {
            reload();
        }
    });
}

Workspace::Workspace(const io::path_t& filePath, const Workspace* other, const modularity::ContextPtr& iocCtx)
    : Workspace(filePath, iocCtx)
{
    m_file = std::make_shared<WorkspaceFile>(filePath, other->m_file.get());
}

std::string Workspace::name() const
{
    return io::completeBasename(m_file->filePath()).toStdString();
}

bool Workspace::isBuiltin() const
{
    io::path_t builtinWorkspacePath = this->builtinWorkspacePath();
    return !builtinWorkspacePath.empty();
}

bool Workspace::isEdited() const
{
    if (!isBuiltin()) {
        return false;
    }

    if (m_file->isNeedSave()) {
        return true;
    }

    return io::absoluteDirpath(filePath()) == configuration()->userWorkspacesPath();
}

RetVal<QByteArray> Workspace::rawData(const DataKey& key) const
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_file->isLoaded()) {
        return RetVal<QByteArray>(make_ret(Err::NotLoaded));
    }

    RetVal<QByteArray> rv;
    rv.ret = make_ret(Ret::Code::Ok);
    rv.val = m_file->data(key);
    return rv;
}

Ret Workspace::setRawData(const DataKey& key, const QByteArray& data)
{
    m_file->setData(key, data);
    return make_ret(Ret::Code::Ok);
}

void Workspace::reset()
{
    if (!isEdited()) {
        return;
    }

    io::path_t builtinWorkspacePath = this->builtinWorkspacePath();

    if (builtinWorkspacePath == filePath()) {
        //! if the user made changes and they were not saved, then just reload file
        reload();
        return;
    }

    //! remove current file
    fileSystem()->remove(filePath());

    //! redirect to builtin workspace file
    m_file = std::make_shared<WorkspaceFile>(builtinWorkspacePath);

    reload();
}

void Workspace::assignNewName(const std::string& newName)
{
    io::path_t filePath = m_file->filePath();
    io::path_t newPath = io::absoluteDirpath(filePath) + "/" + newName + "." + io::suffix(filePath);

    if (filePath == newPath) {
        return;
    }

    Ret ret = doSave();
    if (!ret) {
        LOGE() << "Failed to save workspace, error: " << ret.toString();
    }

    fileSystem()->move(filePath, newPath);

    m_file = std::make_shared<WorkspaceFile>(newPath);
    load();
}

async::Notification Workspace::reloadNotification()
{
    return m_reloadNotification;
}

bool Workspace::isLoaded() const
{
    return m_file->isLoaded();
}

io::path_t Workspace::filePath() const
{
    return m_file->filePath();
}

Ret Workspace::load()
{
    mi::ReadResourceLockGuard resource_guard(multiInstancesProvider.get(), fileResourceName());
    return m_file->load();
}

Ret Workspace::save()
{
    if (isBuiltin()) {
        copyBuiltinWorkspaceToUserDir();
    }

    return doSave();
}

Ret Workspace::doSave()
{
    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider.get(), fileResourceName());
    m_file->setMeta("app_version", Val(application()->version().toStdString()));
    return m_file->save();
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

io::path_t Workspace::builtinWorkspacePath() const
{
    std::string currentWorkspaceName = name();

    io::paths_t builtinWorkspacesPaths = configuration()->builtinWorkspacesFilePaths();
    for (const io::path_t& builtinWorkspacePath : builtinWorkspacesPaths) {
        std::string builtinWorkspaceName = io::completeBasename(builtinWorkspacePath).toStdString();
        if (builtinWorkspaceName == currentWorkspaceName) {
            return builtinWorkspacePath;
        }
    }

    return {};
}

void Workspace::copyBuiltinWorkspaceToUserDir()
{
    Ret ret = doSave();
    if (!ret) {
        LOGE() << "Failed to save builtin workspace, error: " << ret.toString();
    }

    io::path_t userFilePath = configuration()->userWorkspacesPath() + "/" + io::filename(filePath());

    fileSystem()->copy(m_file->filePath(), userFilePath);

    m_file = std::make_shared<WorkspaceFile>(userFilePath);
    load();
}
