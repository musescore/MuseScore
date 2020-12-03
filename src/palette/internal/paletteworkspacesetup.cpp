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

void PaletteWorkspaceSetup::setup()
{
    using namespace Ms;

    if (!workspaceManager()) {
        return;
    }

    Ms::PaletteWorkspace* paletteWorkspace = adapter()->paletteWorkspace();
    auto applyWorkspaceData = [paletteWorkspace](std::shared_ptr<workspace::IWorkspace> w) {
                                  std::shared_ptr<workspace::AbstractData> data = w->data("PaletteBox");
                                  if (!data) {
                                      LOGE() << "no palette data in workspace: " << w->name();
                                      return false;
                                  }

                                  PaletteWorkspaceData* pdata = dynamic_cast<PaletteWorkspaceData*>(data.get());
                                  IF_ASSERT_FAILED(pdata) {
                                      return false;
                                  }

                                  paletteWorkspace->setDefaultPaletteTree(std::move(pdata->tree));
                                  return true;
                              };

    RetValCh<std::shared_ptr<workspace::IWorkspace> > workspace = workspaceManager()->currentWorkspace();
    if (workspace.val) {
        bool ok = applyWorkspaceData(workspace.val);
        if (!ok) {
            std::unique_ptr<PaletteTree> tree(PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(std::move(tree));
        }
    }

    workspace.ch.onReceive(nullptr, [paletteWorkspace, applyWorkspaceData](std::shared_ptr<workspace::IWorkspace> w) {
        bool ok = applyWorkspaceData(w);
        if (!ok) {
            std::unique_ptr<PaletteTree> tree(PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(std::move(tree));
        }
    });
}
