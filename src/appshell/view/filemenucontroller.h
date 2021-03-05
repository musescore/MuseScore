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
#ifndef MU_NOTATION_FILEMENUCONTROLLER_H
#define MU_NOTATION_FILEMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "userscores/ifilescorecontroller.h"
#include "notation/inotationactionscontroller.h"
#include "notation/notationtypes.h"

namespace mu::appshell {
class FileMenuController : public async::Asyncable
{
    INJECT(appshell, userscores::IFileScoreController, fileController)
    INJECT(appshell, notation::INotationActionsController, notationController)

public:
    FileMenuController();

    async::Channel<std::vector<actions::ActionCode>> actionsAvailableChanged() const;

    bool isNewAvailable() const;
    bool isOpenAvailable() const;
    bool isCloseAvailable() const;
    bool isSaveAvailable(notation::SaveMode saveMode) const;
    bool isImportAvailable() const;
    bool isExportAvailable() const;
    bool isEditInfoAvailable() const;
    bool isPartsAvailable() const;
    bool isPrintAvailable() const;
    bool isQuitAvailable() const;

private:
    async::Channel<std::vector<actions::ActionCode>> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_NOTATION_FILEMENUCONTROLLER_H
