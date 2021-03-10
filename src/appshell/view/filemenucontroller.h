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
#ifndef MU_APPSHELL_FILEMENUCONTROLLER_H
#define MU_APPSHELL_FILEMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "userscores/ifilescorecontroller.h"
#include "notation/inotationactionscontroller.h"
#include "notation/notationtypes.h"
#include "ifilemenucontroller.h"

namespace mu::appshell {
class FileMenuController : public IFileMenuController, public async::Asyncable
{
    INJECT(appshell, userscores::IFileScoreController, fileController)
    INJECT(appshell, notation::INotationActionsController, notationController)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    using AvailableCallback = std::function<bool ()>;
    std::map<actions::ActionCode, AvailableCallback> actions() const;

    bool isNewAvailable() const;
    bool isOpenAvailable() const;
    bool isClearRecentAvailable() const;
    bool isCloseAvailable() const;
    bool isSaveAvailable() const;
    bool isSaveAsAvailable() const;
    bool isSaveCopyAvailable() const;
    bool isSaveSelectionAvailable() const;
    bool isSaveOnlineAvailable() const;
    bool isImportAvailable() const;
    bool isExportAvailable() const;
    bool isEditInfoAvailable() const;
    bool isPartsAvailable() const;
    bool isPrintAvailable() const;
    bool isQuitAvailable() const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_FILEMENUCONTROLLER_H
