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
#include "paletterootmodel.h"

using namespace mu::palette;

PaletteRootModel::PaletteRootModel(QObject* parent)
    : QObject(parent)
{
    ValCh<bool> enabled = adapter()->paletteEnabled();
    enabled.ch.onReceive(this, [this](bool arg) {
        emit paletteEnabledChanged(arg);
    });

    adapter()->paletteSearchRequested().onNotify(this, [this]() {
        emit paletteSearchRequested();
    });

    adapter()->elementDraggedToScoreView().onNotify(this, [this]() {
        emit elementDraggedToScoreView();
    });
}

bool PaletteRootModel::paletteEnabled() const
{
    return adapter()->paletteEnabled().val;
}

Ms::PaletteWorkspace* PaletteRootModel::paletteWorkspace() const
{
    return adapter()->paletteWorkspace();
}

bool PaletteRootModel::shadowOverlay() const
{
    return m_shadowOverlay;
}
