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
#ifndef MU_PALETTE_PALETTEADAPTERSTUB_H
#define MU_PALETTE_PALETTEADAPTERSTUB_H

#include "palette/ipaletteadapter.h"

namespace mu::palette {
class PaletteAdapterStub : public IPaletteAdapter
{
public:
    QAction* getAction(const char* id) const override;
    QString actionHelp(const char* id) const override;

    void showMasterPalette(const QString&) override;

    // score
    bool isSelected() const override;
    bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) override;

    // qml
    Ms::PaletteWorkspace* paletteWorkspace() const override;
    mu::ValCh<bool> paletteEnabled() const override;
    void setPaletteEnabled(bool arg) override;
    void requestPaletteSearch() override;
    async::Notification paletteSearchRequested() const override;
    void notifyElementDraggedToScoreView() override;
    async::Notification elementDraggedToScoreView() const override;
};
}

#endif // MU_PALETTE_PALETTEADAPTERSTUB_H
