//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "workspacemanager.h"

#include "log.h"

using namespace mu;
using namespace mu::workspace;
using namespace mu::extensions;

static bool containsWorkspace(const IWorkspacePtrList& list, const IWorkspacePtr& workspace)
{
    return std::find(list.cbegin(), list.cend(), workspace) != list.cend();
}

RetValCh<IWorkspacePtr> WorkspaceManager::currentWorkspace() const
{
    RetValCh<IWorkspacePtr> rv;

    if (!m_currentWorkspace) {
        rv.ret = make_ret(Ret::Code::UnknownError);
        rv.val = nullptr;
    } else {
        rv.ret = make_ret(Ret::Code::Ok);
        rv.val = m_currentWorkspace;
    }

    rv.ch = m_currentWorkspaceChanged;

    return rv;
}

RetVal<IWorkspacePtrList> WorkspaceManager::workspaces() const
{
    RetVal<IWorkspacePtrList> result;
    result.ret = make_ret(Ret::Code::Ok);

    for (auto workspace : m_workspaces) {
        result.val.push_back(workspace);
    }

    return result;
}

Ret WorkspaceManager::setWorkspaces(const IWorkspacePtrList& workspaces)
{
    Ret ret = removeMissingWorkspaces(workspaces);

    if (ret) {
        ret = createInexistentWorkspaces(workspaces);
    }

    return ret;
}

Ret WorkspaceManager::removeMissingWorkspaces(const IWorkspacePtrList& newWorkspaceList)
{
    RetVal<IWorkspacePtrList> oldWorkspaceList = workspaces();
    if (!oldWorkspaceList.ret) {
        return oldWorkspaceList.ret;
    }

    for (const IWorkspacePtr& oldWorkspace : oldWorkspaceList.val) {
        if (containsWorkspace(newWorkspaceList, oldWorkspace)) {
            continue;
        }

        Ret ret = removeWorkspace(oldWorkspace);
        if (!ret) {
            return ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret WorkspaceManager::removeWorkspace(const IWorkspacePtr& workspace)
{
    std::string workspaceName = workspace->name();
    if (!canRemoveWorkspace(workspaceName)) {
        return make_ret(Ret::Code::Ok);
    }

    for (auto it = m_workspaces.begin(); it != m_workspaces.end(); ++it) {
        if (it->get()->name() == workspaceName) {
            m_workspaces.erase(it);
            return fileSystem()->remove(it->get()->filePath());
        }
    }

    return make_ret(Ret::Code::Ok);
}

bool WorkspaceManager::canRemoveWorkspace(const std::string& workspaceName) const
{
    return workspaceName != DEFAULT_WORKSPACE_NAME;
}

Ret WorkspaceManager::createInexistentWorkspaces(const IWorkspacePtrList& newWorkspaceList)
{
    RetVal<IWorkspacePtrList> existentWorkspaces = workspaces();
    if (!existentWorkspaces.ret) {
        return existentWorkspaces.ret;
    }

    for (const IWorkspacePtr& workspace : newWorkspaceList) {
        if (containsWorkspace(existentWorkspaces.val, workspace)) {
            continue;
        }

        Ret ret = createWorkspace(workspace);
        if (!ret) {
            return ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret WorkspaceManager::createWorkspace(IWorkspacePtr workspace)
{
    auto writable = std::dynamic_pointer_cast<Workspace>(workspace);
    if (!writable) {
        return make_ret(Ret::Code::Ok);
    }

    Ret ret = writable->write();

    if (ret) {
        m_workspaces.push_back(writable);
    }

    return ret;
}

void WorkspaceManager::init()
{
    Ret ret = fileSystem()->makePath(configuration()->userWorkspacesDirPath());
    if (!ret) {
        LOGE() << ret.toString();
    }

    RetCh<Extension> extensionChanged = extensionsController()->extensionChanged();
    if (extensionChanged.ret) {
        extensionChanged.ch.onReceive(this, [this](const Extension& newExtension) {
            if (newExtension.types.testFlag(Extension::Workspaces)) {
                load();
            }
        });
    }

    configuration()->currentWorkspaceName().ch.onReceive(this, [this](const std::string&) {
        setupCurrentWorkspace();
    });

    load();
}

void WorkspaceManager::load()
{
    m_workspaces.clear();

    io::paths files = findWorkspaceFiles();

    for (const io::path& file : files) {
        auto workspace = std::make_shared<Workspace>(file);
        m_workspaces.push_back(workspace);
    }

    setupCurrentWorkspace();
}

io::paths WorkspaceManager::findWorkspaceFiles() const
{
    io::paths result;
    io::paths dirPaths = configuration()->workspacePaths();

    for (const io::path& dirPath : dirPaths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(dirPath, { "*.workspace" });
        if (!files.ret) {
            LOGE() << files.ret.toString();
            continue;
        }

        result.insert(result.end(), files.val.begin(), files.val.end());
    }

    return result;
}

void WorkspaceManager::setupCurrentWorkspace()
{
    saveCurrentWorkspace();

    std::string workspaceName = configuration()->currentWorkspaceName().val;

    WorkspacePtr workspace = findAndInit(workspaceName);
    if (!workspace) {
        std::string defaultWorkspaceName(DEFAULT_WORKSPACE_NAME);
        LOGW() << "failed get workspace: " << workspaceName << ", will use " << defaultWorkspaceName;
        workspace = findAndInit(defaultWorkspaceName);
    }

    m_currentWorkspace = workspace;
    m_currentWorkspaceChanged.send(workspace);
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

    if (!workspace->isInited()) {
        Ret ret = workspace->read();
        if (!ret) {
            LOGE() << "failed read workspace: " << name;
            return nullptr;
        }
    }

    return workspace;
}

void WorkspaceManager::deinit()
{
    saveCurrentWorkspace();
}

void WorkspaceManager::saveCurrentWorkspace()
{
    if (!m_currentWorkspace) {
        return;
    }

    Ret ret = m_currentWorkspace->write();
    if (!ret) {
        LOGE() << ret.toString();
    }
}
