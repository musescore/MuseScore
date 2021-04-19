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
