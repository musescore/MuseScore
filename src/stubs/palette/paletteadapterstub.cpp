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
#include "paletteadapterstub.h"

#include <QAction>

using namespace mu::palette;

QAction* PaletteAdapterStub::getAction(const char*) const
{
    return new QAction();
}

QString PaletteAdapterStub::actionHelp(const char*) const
{
    return QString();
}

void PaletteAdapterStub::showMasterPalette(const QString&)
{
}

bool PaletteAdapterStub::isSelected() const
{
    return false;
}

bool PaletteAdapterStub::applyPaletteElement(Ms::Element*, Qt::KeyboardModifiers)
{
    return false;
}

Ms::PaletteWorkspace* PaletteAdapterStub::paletteWorkspace() const
{
    return nullptr;
}

mu::ValCh<bool> PaletteAdapterStub::paletteEnabled() const
{
    return mu::ValCh<bool>();
}

void PaletteAdapterStub::setPaletteEnabled(bool)
{
}

void PaletteAdapterStub::requestPaletteSearch()
{
}

mu::async::Notification PaletteAdapterStub::paletteSearchRequested() const
{
    return mu::async::Notification();
}

void PaletteAdapterStub::notifyElementDraggedToScoreView()
{
}

mu::async::Notification PaletteAdapterStub::elementDraggedToScoreView() const
{
    return mu::async::Notification();
}
