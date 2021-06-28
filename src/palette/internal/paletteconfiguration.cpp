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
#include "paletteconfiguration.h"

#include "log.h"
#include "settings.h"

#include "ui/internal/uiengine.h"

using namespace mu::palette;
using namespace mu::framework;
using namespace mu::ui;

static const std::string MODULE_NAME("palette");
static const Settings::Key PALETTE_SCALE(MODULE_NAME, "application/paletteScale");
static const Settings::Key PALETTE_USE_SINGLE(MODULE_NAME, "application/useSinglePalette");

void PaletteConfiguration::init()
{
    settings()->setDefaultValue(PALETTE_SCALE, Val(1.0));
    settings()->setCanBeMannualyEdited(PALETTE_SCALE, true);
    settings()->setDefaultValue(PALETTE_USE_SINGLE, Val(false));
    settings()->setCanBeMannualyEdited(PALETTE_USE_SINGLE, true);
}

double PaletteConfiguration::paletteScaling() const
{
    return settings()->value(PALETTE_SCALE).toDouble();
}

void PaletteConfiguration::setPaletteScaling(double scale)
{
    settings()->setSharedValue(PALETTE_SCALE, Val(scale));
}

bool PaletteConfiguration::isSinglePalette() const
{
    return settings()->value(PALETTE_USE_SINGLE).toBool();
}

void PaletteConfiguration::setIsSinglePalette(bool isSingle)
{
    settings()->setSharedValue(PALETTE_USE_SINGLE, Val(isSingle));
}

QColor PaletteConfiguration::elementsBackgroundColor() const
{
    return themeColor(BACKGROUND_PRIMARY_COLOR);
}

QColor PaletteConfiguration::elementsColor() const
{
    return themeColor(FONT_PRIMARY_COLOR);
}

QColor PaletteConfiguration::gridColor() const
{
    return themeColor(STROKE_COLOR);
}

QColor PaletteConfiguration::accentColor() const
{
    return themeColor(ACCENT_COLOR);
}

QColor PaletteConfiguration::themeColor(ThemeStyleKey key) const
{
    return uiConfiguration()->currentTheme().values[key].toString();
}

mu::async::Notification PaletteConfiguration::colorsChanged() const
{
    return uiConfiguration()->currentThemeChanged();
}

mu::io::path PaletteConfiguration::keySignaturesDirPath() const
{
    return globalConfiguration()->userAppDataPath() + "/keysigs";
}

mu::io::path PaletteConfiguration::timeSignaturesDirPath() const
{
    return globalConfiguration()->userAppDataPath() + "/timesigs";
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
