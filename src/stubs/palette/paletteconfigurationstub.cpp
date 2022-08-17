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
#include "paletteconfigurationstub.h"

using namespace mu::palette;

double PaletteConfigurationStub::paletteScaling() const
{
    return 0.f;
}

void PaletteConfigurationStub::setPaletteScaling(double)
{
}

bool PaletteConfigurationStub::isSinglePalette() const
{
    return false;
}

void PaletteConfigurationStub::setIsSinglePalette(bool)
{
}

QColor PaletteConfigurationStub::elementsBackgroundColor() const
{
    return QColor();
}

QColor PaletteConfigurationStub::elementsColor() const
{
    return QColor();
}

QColor PaletteConfigurationStub::gridColor() const
{
    return QColor();
}

QColor PaletteConfigurationStub::accentColor() const
{
    return QColor();
}

mu::async::Notification PaletteConfigurationStub::colorsChanged() const
{
    return mu::async::Notification();
}

mu::io::path_t PaletteConfigurationStub::keySignaturesDirPath() const
{
    return mu::io::path_t();
}

mu::io::path_t PaletteConfigurationStub::timeSignaturesDirPath() const
{
    return mu::io::path_t();
}

bool PaletteConfigurationStub::useFactorySettings() const
{
    return false;
}

bool PaletteConfigurationStub::enableExperimental() const
{
    return false;
}

mu::ValCh<IPaletteConfiguration::PaletteConfig> PaletteConfigurationStub::paletteConfig(const QString&) const
{
    return mu::ValCh<IPaletteConfiguration::PaletteConfig>();
}

void PaletteConfigurationStub::setPaletteConfig(const QString&, const IPaletteConfiguration::PaletteConfig&)
{
}

mu::ValCh<IPaletteConfiguration::PaletteCellConfig> PaletteConfigurationStub::paletteCellConfig(const QString&) const
{
    return mu::ValCh<IPaletteConfiguration::PaletteCellConfig>();
}

void PaletteConfigurationStub::setPaletteCellConfig(const QString&, const IPaletteConfiguration::PaletteCellConfig&)
{
}
