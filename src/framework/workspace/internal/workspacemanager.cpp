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
#include "workspacemanager.h"

#include "types/uri.h"

#include "log.h"

using namespace muse;
using namespace muse::workspace;

static const std::string WORKSPACE_EXT(".mws");

static bool containsWorkspace(const IWorkspacePtrList& list, const IWorkspacePtr& workspace)
{
    return std::find(list.cbegin(), list.cend(), workspace) != list.cend();
}

void WorkspaceManager::init()
{
    configuration()->currentWorkspaceNameChanged().onReceive(this, [this](const std::string&) {
        setupCurrentWorkspace();
    });

    load();
}

void WorkspaceManager::deinit()
{
    saveCurrentWorkspace();

    m_currentWorkspace = nullptr;
    m_defaultWorkspace = nullptr;
    m_workspaces.clear();
}

bool WorkspaceManager::isInited() const
{
    return m_defaultWorkspace != nullptr;
}

IWorkspacePtr WorkspaceManager::defaultWorkspace() const
{
    return m_defaultWorkspace;
}

IWorkspacePtr WorkspaceManager::currentWorkspace() const
{
    return m_currentWorkspace;
}

async::Notification WorkspaceManager::currentWorkspaceAboutToBeChanged() const
{
    return m_currentWorkspaceAboutToBeChanged;
}

async::Notification WorkspaceManager::currentWorkspaceChanged() const
{
    return m_currentWorkspaceChanged;
}

IWorkspacePtrList WorkspaceManager::workspaces() const
{
    IWorkspacePtrList iworkspaces;
    iworkspaces.reserve(m_workspaces.size());
    for (WorkspacePtr w : m_workspaces) {
        iworkspaces.push_back(w);
    }
    return iworkspaces;
}

Ret WorkspaceManager::setWorkspaces(const IWorkspacePtrList& workspaces)
{
    Ret ret = removeMissingWorkspaces(workspaces);

    if (ret) {
        ret = addNonExistentWorkspaces(workspaces);
    }

    m_workspacesListChanged.notify();

    return ret;
}

async::Notification WorkspaceManager::workspacesListChanged() const
{
    return m_workspacesListChanged;
}

IWorkspacePtr WorkspaceManager:: cloneWorkspace(const IWorkspacePtr& workspace, const std::string& newWorkspaceName) const
{
    return std::make_shared<Workspace>(makeNewWorkspacePath(newWorkspaceName), dynamic_cast<Workspace*>(workspace.get()), iocContext());
}

void WorkspaceManager::changeCurrentWorkspace(const std::string& newWorkspaceName)
{
    if (configuration()->currentWorkspaceName() == newWorkspaceName || newWorkspaceName.empty()) {
        return;
    }

    prepareCurrentWorkspaceForChange();

    configuration()->setCurrentWorkspaceName(newWorkspaceName);
}

void WorkspaceManager::createAndAppendNewWorkspace()
{
    prepareCurrentWorkspaceForChange();

    IWorkspacePtrList workspaces = this->workspaces();

    QStringList workspaceNames;
    for (const IWorkspacePtr& workspace: workspaces) {
        workspaceNames << QString::fromStdString(workspace->name());
    }

    UriQuery uri("muse://workspace/create");
    uri.addParam("sync", Val(true));
    uri.addParam("workspaceNames", Val(workspaceNames.join(',')));

    RetVal<Val> obj = interactive()->open(uri);
    if (!obj.ret) {
        return;
    }

    QVariantMap meta = obj.val.toQVariant().toMap();
    QString name = meta.value("name").toString();
    IF_ASSERT_FAILED(!name.isEmpty()) {
        return;
    }

    IWorkspacePtr newWorkspace = cloneWorkspace(currentWorkspace(), name.toStdString());
    if (!newWorkspace) {
        return;
    }

    workspaces.emplace_back(newWorkspace);

    setWorkspaces(workspaces);

    configuration()->setCurrentWorkspaceName(name.toStdString());
}

void WorkspaceManager::openConfigureWorkspacesDialog()
{
    prepareCurrentWorkspaceForChange();

    RetVal<Val> result = interactive()->open("muse://workspace/select?sync=true");
    if (!result.ret) {
        return;
    }

    std::string selectedWorkspace = result.val.toString();
    changeCurrentWorkspace(selectedWorkspace);
}

WorkspacePtr WorkspaceManager::doNewWorkspace(const std::string& workspaceName) const
{
    return std::make_shared<Workspace>(makeNewWorkspacePath(workspaceName), iocContext());
}

io::path_t WorkspaceManager::makeNewWorkspacePath(const std::string& workspaceName) const
{
    return configuration()->userWorkspacesPath() + "/" + workspaceName + WORKSPACE_EXT;
}

void WorkspaceManager::appendNewWorkspace(WorkspacePtr workspace)
{
    setupConnectionsToNewWorkspace(workspace);
    m_workspaces.push_back(workspace);
}

void WorkspaceManager::setupConnectionsToNewWorkspace(const IWorkspacePtr workspace)
{
    std::string newWorkspaceName = workspace->name();
    workspace->reloadNotification().onNotify(this, [this, newWorkspaceName](){
        if (m_currentWorkspace->name() == newWorkspaceName) {
            m_currentWorkspaceChanged.notify();
        }
    });
}

Ret WorkspaceManager::removeMissingWorkspaces(const IWorkspacePtrList& newWorkspaceList)
{
    IWorkspacePtrList oldWorkspaceList = workspaces();
    for (const IWorkspacePtr& oldWorkspace : oldWorkspaceList) {
        if (containsWorkspace(newWorkspaceList, oldWorkspace)) {
            continue;
        }

        Ret ret = removeWorkspace(oldWorkspace);
        if (!ret) {
            return ret;
        }
    }

    if (!containsWorkspace(newWorkspaceList, m_currentWorkspace)) {
        m_currentWorkspace = nullptr;
    }

    return make_ret(Ret::Code::Ok);
}

Ret WorkspaceManager::removeWorkspace(const IWorkspacePtr& workspace)
{
    if (!canRemoveWorkspace(workspace)) {
        return make_ret(Ret::Code::Ok);
    }

    std::string workspaceName = workspace->name();
    for (auto it = m_workspaces.begin(); it != m_workspaces.end(); ++it) {
        if (it->get()->name() == workspaceName) {
            Ret ret = fileSystem()->remove(it->get()->filePath());
            if (ret) {
                m_workspaces.erase(it);
            }

            return ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}

bool WorkspaceManager::canRemoveWorkspace(const IWorkspacePtr& workspace) const
{
    return !workspace->isBuiltin();
}

Ret WorkspaceManager::addNonExistentWorkspaces(const IWorkspacePtrList& newWorkspaceList)
{
    IWorkspacePtrList existentWorkspaces = workspaces();
    for (const IWorkspacePtr& workspace : newWorkspaceList) {
        if (containsWorkspace(existentWorkspaces, workspace)) {
            continue;
        }

        Ret ret = addWorkspace(workspace);
        if (!ret) {
            return ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret WorkspaceManager::addWorkspace(IWorkspacePtr workspace)
{
    auto writable = std::dynamic_pointer_cast<Workspace>(workspace);
    if (!writable) {
        return make_ret(Ret::Code::Ok);
    }

    Ret ret = writable->save();

    if (ret) {
        appendNewWorkspace(writable);
    }

    return ret;
}

void WorkspaceManager::load()
{
    m_workspaces.clear();

    io::paths_t files = findWorkspaceFiles();
    for (const io::path_t& file : files) {
        auto workspace = std::make_shared<Workspace>(file, iocContext());
        appendNewWorkspace(workspace);
    }

    m_workspacesListChanged.notify();

    setupDefaultWorkspace();
    setupCurrentWorkspace();
}

io::paths_t WorkspaceManager::findWorkspaceFiles() const
{
    auto findFiles = [this](const io::path_t& dirPath) {
        std::string filter = "*" + WORKSPACE_EXT;
        RetVal<io::paths_t> files = fileSystem()->scanFiles(dirPath, { filter });
        if (!files.ret) {
            LOGE() << files.ret.toString();
        }

        return files.val;
    };

    io::paths_t builtinWorkspacesPaths = configuration()->builtinWorkspacesFilePaths();
    std::set<io::path_t> builtinWorkspacesFileNames;
    for (const io::path_t& dirPath : builtinWorkspacesPaths) {
        builtinWorkspacesFileNames.insert(io::filename(dirPath));
    }

    io::paths_t userWorkspacesPaths = findFiles(configuration()->userWorkspacesPath());
    muse::remove_if(userWorkspacesPaths, [=](const io::path_t& path) -> bool {
        io::path_t fileName = io::filename(path);
        return muse::contains(builtinWorkspacesFileNames, fileName);
    });

    io::paths_t result;
    muse::join(result, builtinWorkspacesPaths);
    muse::join(result, userWorkspacesPaths);

    return result;
}

void WorkspaceManager::setupDefaultWorkspace()
{
    std::string defaultWorkspaceName = configuration()->defaultWorkspaceName();
    WorkspacePtr workspace = findAndInit(defaultWorkspaceName);
    if (workspace) {
        m_defaultWorkspace = workspace;
        return;
    }

    LOGW() << "not found default workspace, will be created new";

    m_defaultWorkspace = doNewWorkspace(defaultWorkspaceName);
    m_workspaces.push_back(m_defaultWorkspace);

    Ret ret = fileSystem()->makePath(configuration()->userWorkspacesPath());
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    ret = m_defaultWorkspace->load();
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }
}

void WorkspaceManager::setupCurrentWorkspace()
{
    std::string workspaceName = configuration()->currentWorkspaceName();
    if (m_currentWorkspace && m_currentWorkspace->isLoaded()) {
        if (m_currentWorkspace->name() == workspaceName) {
            return;
        }

        saveCurrentWorkspace();

        //! NOTE Perhaps we need to unload the current workspace (clear memory)
    }

    WorkspacePtr workspace = findAndInit(workspaceName);
    if (!workspace) {
        std::string defaultWorkspaceName = configuration()->defaultWorkspaceName();
        LOGW() << "failed get workspace: " << workspaceName << ", will use " << defaultWorkspaceName;

        //! NOTE Already should be inited
        IF_ASSERT_FAILED(m_defaultWorkspace) {
            setupDefaultWorkspace();
        }

        workspace = m_defaultWorkspace;
        configuration()->setCurrentWorkspaceName(defaultWorkspaceName);
    }

    m_currentWorkspace = workspace;
    m_currentWorkspaceChanged.notify();
}

WorkspacePtr WorkspaceManager::findByName(const std::string& name) const
{
    for (auto workspace : m_workspaces) {
        if (workspace->name() == name) {
            return workspace;
        }
    }

    return nullptr;
}

WorkspacePtr WorkspaceManager::findAndInit(const std::string& name) const
{
    WorkspacePtr workspace = findByName(name);
    if (!workspace) {
        return nullptr;
    }

    if (!workspace->isLoaded()) {
        Ret ret = workspace->load();
        if (!ret) {
            LOGE() << "failed load workspace: " << name;
            return nullptr;
        }
    }

    return workspace;
}

void WorkspaceManager::saveCurrentWorkspace()
{
    if (!m_currentWorkspace) {
        return;
    }

    Ret ret = m_currentWorkspace->save();
    if (!ret) {
        LOGE() << "failed save current workspace, err: " << ret.toString();
    }
}

void WorkspaceManager::prepareCurrentWorkspaceForChange()
{
    m_currentWorkspaceAboutToBeChanged.notify();
}
