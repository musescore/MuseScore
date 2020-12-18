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
#include "paletteconfiguration.h"

#include "log.h"
#include "settings.h"

#include "ui/internal/uiengine.h"

using namespace mu::palette;
using namespace mu::framework;

static const std::string MODULE_NAME("palette");
static const Settings::Key PALETTE_SCALE(MODULE_NAME, "application/paletteScale");
static const Settings::Key PALETTE_USE_SINGLE(MODULE_NAME, "application/useSinglePalette");

double PaletteConfiguration::paletteScaling() const
{
    double pref = 1.0;
    Val val = settings()->value(PALETTE_SCALE);
    if (!val.isNull()) {
        pref = val.toDouble();
    }

    return pref;
}

bool PaletteConfiguration::isSinglePalette() const
{
    return settings()->value(PALETTE_USE_SINGLE).toBool();
}

QColor PaletteConfiguration::elementsBackgroundColor() const
{
    return theme()->backgroundPrimaryColor();
}

QColor PaletteConfiguration::elementsColor() const
{
    return theme()->fontPrimaryColor();
}

QColor PaletteConfiguration::gridColor() const
{
    return theme()->strokeColor();
}

QColor PaletteConfiguration::accentColor() const
{
    return theme()->accentColor();
}

mu::async::Notification PaletteConfiguration::colorsChanged() const
{
    return theme()->themeChanged();
}

mu::io::path PaletteConfiguration::keySignaturesDirPath() const
{
    return globalConfiguration()->dataPath() + "/keysigs";
}

mu::io::path PaletteConfiguration::timeSignaturesDirPath() const
{
    return globalConfiguration()->dataPath() + "/timesigs";
}

bool PaletteConfiguration::useFactorySettings() const
{
    return globalConfiguration()->useFactorySettings();
}

bool PaletteConfiguration::enableExperimental() const
{
    return globalConfiguration()->enableExperimental();
}

mu::ValCh<PaletteConfiguration::PaletteConfig> PaletteConfiguration::paletteConfig(const QString& paletteId) const
{
    if (!m_paletteConfigs.contains(paletteId)) {
        m_paletteConfigs[paletteId] = ValCh<PaletteConfig>();
    }

    return m_paletteConfigs[paletteId];
}

void PaletteConfiguration::setPaletteConfig(const QString& paletteId, const PaletteConfig& config)
{
    m_paletteConfigs[paletteId].set(config);
}

mu::ValCh<PaletteConfiguration::PaletteCellConfig> PaletteConfiguration::paletteCellConfig(const QString& cellId) const
{
    if (!m_paletteCellsConfigs.contains(cellId)) {
        m_paletteCellsConfigs[cellId] = ValCh<PaletteCellConfig>();
    }

    return m_paletteCellsConfigs[cellId];
}

void PaletteConfiguration::setPaletteCellConfig(const QString& cellId, const PaletteCellConfig& config)
{
    m_paletteCellsConfigs[cellId].set(config);
}
