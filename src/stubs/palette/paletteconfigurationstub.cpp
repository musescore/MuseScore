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
#include "paletteconfigurationstub.h"

using namespace mu::palette;

double PaletteConfigurationStub::paletteScaling() const
{
    return 0.f;
}

bool PaletteConfigurationStub::isSinglePalette() const
{
    return false;
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

mu::io::path PaletteConfigurationStub::keySignaturesDirPath() const
{
    return mu::io::path();
}

mu::io::path PaletteConfigurationStub::timeSignaturesDirPath() const
{
    return mu::io::path();
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
