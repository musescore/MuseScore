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
#ifndef MU_APPSHELL_TOOLSMENUCONTROLLER_H
#define MU_APPSHELL_TOOLSMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "notation/inotationactionscontroller.h"

#include "itoolsmenucontroller.h"

namespace mu::appshell {
class ToolsMenuController : public IToolsMenuController, public async::Asyncable
{
    INJECT(appshell, notation::INotationActionsController, controller)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    using AvailableCallback = std::function<bool ()>;
    std::map<actions::ActionCode, AvailableCallback> actions() const;

    bool isVoiceAvailable(int voiceIndex1, int voiceIndex2) const;
    bool isSplitMeasureAvailable() const;
    bool isJoinMeasuresAvailable() const;
    bool isTransposeAvailable() const;
    bool isExplodeAvailable() const;
    bool isImplodeAvailable() const;
    bool isRealizeSelectedChordSymbolsAvailable() const;
    bool isRemoveSelectedRangeAvailable() const;
    bool isRemoveEmptyTrailingMeasuresAvailable() const;
    bool isFillSelectionWithSlashesAvailable() const;
    bool isReplaceSelectedNotesWithSlashesAvailable() const;
    bool isSpellPitchesAvailable() const;
    bool isRegroupNotesAndRestsAvailable() const;
    bool isResequenceRehearsalMarksAvailable() const;
    bool isUnrollRepeatsAvailable() const;
    bool isCopyLyricsAvailable() const;
    bool isFotomodeAvailable() const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_TOOLSMENUCONTROLLER_H
