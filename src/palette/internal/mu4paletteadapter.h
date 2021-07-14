/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_PALETTE_MU4PALETTEADAPTER_H
#define MU_PALETTE_MU4PALETTEADAPTER_H

#include "ipaletteadapter.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace mu::palette {
class MU4PaletteAdapter : public IPaletteAdapter
{
    INJECT(palette, framework::IInteractive, interactive)

public:
    MU4PaletteAdapter();

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
