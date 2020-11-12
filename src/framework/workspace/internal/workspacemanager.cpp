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

#include <QDir>

#include "log.h"

using namespace mu;
using namespace mu::workspace;
using namespace mu::extensions;

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

RetValCh<IWorkspacePtrList> WorkspaceManager::allWorkspaces() const
{
    RetValCh<IWorkspacePtrList> result;

    result.ret = make_ret(Ret::Code::Ok);
    result.ch = m_workspacesChanged;

    for (auto workspace : m_workspaces) {
        result.val.push_back(workspace);
    }

    return result;
}

void WorkspaceManager::init()
{
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
    IWorkspacePtrList workspaces;

    for (const io::path& file : files) {
        auto workspace = std::make_shared<Workspace>(file);
        m_workspaces.push_back(workspace);
        workspaces.push_back(workspace);
    }

    setupCurrentWorkspace();

    m_workspacesChanged.send(workspaces);
}

io::paths WorkspaceManager::findWorkspaceFiles() const
{
    io::paths files;
    io::paths paths = configuration()->workspacePaths();

    for (const io::path& path : paths) {
        //! TODO Change on use IFileSystem
        QStringList flist = QDir(path.toQString()).entryList({ "*.workspace" }, QDir::Files);
        for (const QString& f : flist) {
            files.push_back(path + "/" + f);
        }
    }
    return files;
}

void WorkspaceManager::setupCurrentWorkspace()
{
    std::string workspaceName = configuration()->currentWorkspaceName().val;

    WorkspacePtr workspace = findAndInit(workspaceName);
    if (!workspace) {
        LOGW() << "filed get workspace: " << workspaceName << ", will use Default";
        workspace = findAndInit(std::string(DEFAULT_WORKSPACE_NAME));
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
