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
using namespace mu::actions;

void FileMenuController::init()
{
    fileController()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    notationController()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool FileMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool FileMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<ActionCodeList> FileMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<ActionCode, FileMenuController::AvailableCallback> FileMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "file-new", std::bind(&FileMenuController::isNewAvailable, this) },
        { "file-open", std::bind(&FileMenuController::isOpenAvailable, this) },
        { "clear-recent", std::bind(&FileMenuController::isClearRecentAvailable, this) },
        { "file-close", std::bind(&FileMenuController::isCloseAvailable, this) },
        { "file-save", std::bind(&FileMenuController::isSaveAvailable, this) },
        { "file-save-as", std::bind(&FileMenuController::isSaveAsAvailable, this) },
        { "file-save-a-copy", std::bind(&FileMenuController::isSaveCopyAvailable, this) },
        { "file-save-selection", std::bind(&FileMenuController::isSaveSelectionAvailable, this) },
        { "file-save-online", std::bind(&FileMenuController::isSaveOnlineAvailable, this) },
        { "file-import-pdf", std::bind(&FileMenuController::isImportAvailable, this) },
        { "file-export", std::bind(&FileMenuController::isExportAvailable, this) },
        { "edit-info", std::bind(&FileMenuController::isEditInfoAvailable, this) },
        { "parts", std::bind(&FileMenuController::isPartsAvailable, this) },
        { "print", std::bind(&FileMenuController::isPrintAvailable, this) },
        { "quit", std::bind(&FileMenuController::isQuitAvailable, this) }
    };

    return _actions;
}

bool FileMenuController::isNewAvailable() const
{
    return fileController()->actionAvailable("file-new");
}

bool FileMenuController::isOpenAvailable() const
{
    return fileController()->actionAvailable("file-open");
}

bool FileMenuController::isClearRecentAvailable() const
{
    return fileController()->actionAvailable("clear-recent");
}

bool FileMenuController::isCloseAvailable() const
{
    return fileController()->actionAvailable("file-close");
}

bool FileMenuController::isSaveAvailable() const
{
    return fileController()->actionAvailable("file-save");
}

bool FileMenuController::isSaveAsAvailable() const
{
    return fileController()->actionAvailable("file-save-as");
}

bool FileMenuController::isSaveCopyAvailable() const
{
    return fileController()->actionAvailable("file-save-a-copy");
}

bool FileMenuController::isSaveSelectionAvailable() const
{
    return fileController()->actionAvailable("file-save-selection");
}

bool FileMenuController::isSaveOnlineAvailable() const
{
    return fileController()->actionAvailable("file-save-online");
}

bool FileMenuController::isImportAvailable() const
{
    return fileController()->actionAvailable("file-import-pdf");
}

bool FileMenuController::isExportAvailable() const
{
    return fileController()->actionAvailable("file-export");
}

bool FileMenuController::isEditInfoAvailable() const
{
    return notationController()->actionAvailable("edit-info");
}

bool FileMenuController::isPartsAvailable() const
{
    return fileController()->actionAvailable("parts");
}

bool FileMenuController::isPrintAvailable() const
{
    return fileController()->actionAvailable("print");
}

bool FileMenuController::isQuitAvailable() const
{
    return fileController()->actionAvailable("quit");
}
