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
#include "paletteworkspacesetup.h"

#include "log.h"
#include "workspacepalettestream.h"
#include "palette/paletteworkspace.h"
#include "palette/palettecreator.h"

using namespace mu::palette;
using namespace Ms;

void PaletteWorkspaceSetup::setup()
{
    if (!workspaceManager()) {
        return;
    }

    PaletteWorkspace* paletteWorkspace = adapter()->paletteWorkspace();
    auto updateWorkspaceConnection = std::make_shared<QMetaObject::Connection>();

    auto applyWorkspaceData = [paletteWorkspace, updateWorkspaceConnection](workspace::IWorkspacePtr workspace) {
        workspace::AbstractDataPtr data = workspace->data(workspace::WorkspaceTag::Palettes);
        PaletteWorkspaceDataPtr palette = std::dynamic_pointer_cast<PaletteWorkspaceData>(data);

        if (!palette) {
            LOGW() << "no palette data in workspace: " << workspace->name();
            return false;
        }

        paletteWorkspace->setDefaultPaletteTree(palette->tree);
        paletteWorkspace->setUserPaletteTree(palette->tree);

        if (updateWorkspaceConnection) {
            QObject::disconnect(*updateWorkspaceConnection);
        }

        auto newConnection
            = QObject::connect(paletteWorkspace, &PaletteWorkspace::userPaletteChanged, [workspace, palette]() {
            workspace->addData(palette);
        });

        *updateWorkspaceConnection = newConnection;

        return true;
    };

    RetValCh<workspace::IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (workspace.val) {
        bool ok = applyWorkspaceData(workspace.val);
        if (!ok) {
            Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(tree);
        }
    }

    workspace.ch.onReceive(nullptr, [paletteWorkspace, applyWorkspaceData](workspace::IWorkspacePtr w) {
        bool ok = applyWorkspaceData(w);
        if (!ok) {
            Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(tree);
        }
    });
}
