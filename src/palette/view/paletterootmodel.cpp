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
#include "paletterootmodel.h"

using namespace mu::palette;

PaletteRootModel::PaletteRootModel(QObject* parent)
    : QObject(parent)
{
    dispatcher()->reg(this, "palette-search", [this]() {
        emit paletteSearchRequested();
    });
}

bool PaletteRootModel::paletteEnabled() const
{
    // TODO?
    return true;
}

Ms::PaletteProvider* PaletteRootModel::paletteProvider_property() const
{
    return dynamic_cast<Ms::PaletteProvider*>(paletteProvider().get());
}

bool PaletteRootModel::needShowShadowOverlay() const
{
    return m_needShowShadowOverlay;
}
