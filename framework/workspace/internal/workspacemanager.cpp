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

static const std::string WORKSPACE_BASIC("Basic");
static const std::string WORKSPACE_ADVANCED("Advanced");

RetValCh<std::shared_ptr<IWorkspace> > WorkspaceManager::currentWorkspace() const
{
    RetValCh<std::shared_ptr<IWorkspace> > rv;
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

    load();
}

void WorkspaceManager::load()
{
    m_workspaces.clear();

    std::vector<io::path> files = findWorkspaceFiles();
    for (const io::path& f : files) {
        auto w = std::make_shared<Workspace>(f);
        m_workspaces.push_back(w);
    }

    setupCurrentWorkspace();
}

std::vector<io::path> WorkspaceManager::findWorkspaceFiles() const
{
    std::vector<io::path> files;
    std::vector<io::path> paths = configuration()->workspacePaths();
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
    std::string wsname = configuration()->currentWorkspaceName();

    std::shared_ptr<Workspace> w = findAndInit(wsname);
    if (!w) {
        LOGW() << "filed get workspace: " << wsname << ", will use Basic";
        w = findAndInit(WORKSPACE_BASIC);
    }

//    IF_ASSERT_FAILED(w) {
//        return;
//    }

    m_currentWorkspace = w;
    m_currentWorkspaceChanged.send(w);
}

std::shared_ptr<Workspace> WorkspaceManager::findByName(const std::string& name) const
{
    for (auto w : m_workspaces) {
        if (w->name() == name) {
            return w;
        }
    }
    return nullptr;
}

std::shared_ptr<Workspace> WorkspaceManager::findAndInit(const std::string& name) const
{
    std::shared_ptr<Workspace> w = findByName(name);
    if (!w) {
        return nullptr;
    }

    if (!w->isInited()) {
        Ret ret = w->read();
        if (!ret) {
            LOGE() << "failed read workspace: " << name;
            return nullptr;
        }
    }

    return w;
}
