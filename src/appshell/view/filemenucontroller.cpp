//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "filemenucontroller.h"

using namespace mu::appshell;
using namespace mu::notation;

FileMenuController::FileMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > FileMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

bool FileMenuController::isNewAvailable() const
{
    return controller()->actionAvailable("file-new");
}

bool FileMenuController::isOpenAvailable() const
{
    return controller()->actionAvailable("file-open");
}

bool FileMenuController::isCloseAvailable() const
{
    return controller()->actionAvailable("file-close");
}

bool FileMenuController::isSaveAvailable(mu::notation::SaveMode saveMode) const
{
    return controller()->actionAvailable("file-save-" + saveModeToString(saveMode));
}

bool FileMenuController::isImportAvailable() const
{
    return controller()->actionAvailable("file-import-pdf");
}

bool FileMenuController::isExportAvailable() const
{
    return controller()->actionAvailable("file-export");
}

bool FileMenuController::isEditInfoAvailable() const
{
    return controller()->actionAvailable("edit-info");
}

bool FileMenuController::isPartsAvailable() const
{
    return controller()->actionAvailable("parts");
}

bool FileMenuController::isPrintAvailable() const
{
    return controller()->actionAvailable("print");
}

bool FileMenuController::isQuitAvailable() const
{
    return controller()->actionAvailable("quit");
}
