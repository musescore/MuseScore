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
#ifndef MU_PALETTE_MU4PALETTEADAPTER_H
#define MU_PALETTE_MU4PALETTEADAPTER_H

#include <QHash>
#include "../ipaletteadapter.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"
#include "ui/iuiactionsregister.h"

namespace mu::palette {
class MU4PaletteAdapter : public IPaletteAdapter
{
    INJECT(palette, context::IGlobalContext, globalContext)
    INJECT(palette, framework::IInteractive, interactive)
    INJECT(palette, ui::IUiActionsRegister, actionsRegister)

public:
    MU4PaletteAdapter();

    const ui::UiAction& getAction(const actions::ActionCode& code) const override;

    void showMasterPalette(const QString& selectedPaletteName) override;
    bool isSelected() const override;

    // score view
    bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) override;
    // ---

    // qml
    Ms::PaletteWorkspace* paletteWorkspace() const override;
    ValCh<bool> paletteEnabled() const override;
    void setPaletteEnabled(bool arg) override;
    void requestPaletteSearch() override;
    mu::async::Notification paletteSearchRequested() const override;
    void notifyElementDraggedToScoreView() override;
    mu::async::Notification elementDraggedToScoreView() const override;

private:
    ValCh<bool> m_paletteEnabled;
    mutable Ms::PaletteWorkspace* m_paletteWorkspace = nullptr;
    mu::async::Notification m_paletteSearchRequested;
    mu::async::Notification m_elementDraggedToScoreView;
};
}

#endif // MU_PALETTE_MU4PALETTEADAPTER_H
