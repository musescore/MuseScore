/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "translation.h"

#include "ui/internal/uiengine.h"

using namespace mu;
using namespace mu::palette;
using namespace muse;
using namespace muse::ui;

static const std::string MODULE_NAME("palette");
static const Settings::Key PALETTE_SCALE(MODULE_NAME, "application/paletteScale");
static const Settings::Key PALETTE_USE_SINGLE(MODULE_NAME, "application/useSinglePalette");
static const Settings::Key IS_SINGLE_CLICK_TO_OPEN_PALETTE(MODULE_NAME, "application/singleClickToOpenPalette");
static const Settings::Key IS_PALETTE_DRAG_ENABLED(MODULE_NAME, "application/paletteDragEnabled");

void PaletteConfiguration::init()
{
    settings()->setDefaultValue(PALETTE_SCALE, Val(1.0));
    settings()->setDescription(PALETTE_SCALE, muse::qtrc("palette", "Palette scale").toStdString());
    settings()->setCanBeManuallyEdited(PALETTE_SCALE, true, Val(0.5), Val(5.0));

    settings()->setDefaultValue(PALETTE_USE_SINGLE, Val(false));

    m_isSinglePalette.val = settings()->value(PALETTE_USE_SINGLE).toBool();
    settings()->valueChanged(PALETTE_USE_SINGLE).onReceive(this, [this](const Val& newValue) {
        m_isSinglePalette.set(newValue.toBool());
    });

    settings()->setDefaultValue(IS_SINGLE_CLICK_TO_OPEN_PALETTE, Val(true));

    m_isSingleClickToOpenPalette.val = settings()->value(IS_SINGLE_CLICK_TO_OPEN_PALETTE).toBool();
    settings()->valueChanged(IS_SINGLE_CLICK_TO_OPEN_PALETTE).onReceive(this, [this](const Val& newValue) {
        m_isSingleClickToOpenPalette.set(newValue.toBool());
    });

    settings()->setDefaultValue(IS_PALETTE_DRAG_ENABLED, Val(true));

    m_isPaletteDragEnabled.val = settings()->value(IS_PALETTE_DRAG_ENABLED).toBool();
    settings()->valueChanged(IS_PALETTE_DRAG_ENABLED).onReceive(this, [this](const Val& newValue) {
        m_isPaletteDragEnabled.set(newValue.toBool());
    });
}

double PaletteConfiguration::paletteScaling() const
{
    return settings()->value(PALETTE_SCALE).toDouble();
}

void PaletteConfiguration::setPaletteScaling(double scale)
{
    settings()->setSharedValue(PALETTE_SCALE, Val(scale));
}

double PaletteConfiguration::paletteSpatium() const
{
    static constexpr const double DPI_DISPLAY = 96.0;  // 96 DPI nominal resolution
    static constexpr const double DPMM_DISPLAY = DPI_DISPLAY / 25.4;
    static constexpr const double PALETTE_SPATIUM = 1.764 * DPMM_DISPLAY;

    return PALETTE_SPATIUM;
}

ValCh<bool> PaletteConfiguration::isSinglePalette() const
{
    return m_isSinglePalette;
}

void PaletteConfiguration::setIsSinglePalette(bool isSingle)
{
    settings()->setSharedValue(PALETTE_USE_SINGLE, Val(isSingle));
}

ValCh<bool> PaletteConfiguration::isSingleClickToOpenPalette() const
{
    return m_isSingleClickToOpenPalette;
}

void PaletteConfiguration::setIsSingleClickToOpenPalette(bool isSingleClick)
{
    settings()->setSharedValue(IS_SINGLE_CLICK_TO_OPEN_PALETTE, Val(isSingleClick));
}

ValCh<bool> PaletteConfiguration::isPaletteDragEnabled() const
{
    return m_isPaletteDragEnabled;
}

void PaletteConfiguration::setIsPaletteDragEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PALETTE_DRAG_ENABLED, Val(enabled));
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

muse::async::Notification PaletteConfiguration::colorsChanged() const
{
    return uiConfiguration()->currentThemeChanged();
}

muse::io::path_t PaletteConfiguration::keySignaturesDirPath() const
{
    return globalConfiguration()->userAppDataPath() + "/keysigs";
}

muse::io::path_t PaletteConfiguration::timeSignaturesDirPath() const
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

ValCh<PaletteConfiguration::PaletteConfig> PaletteConfiguration::paletteConfig(const QString& paletteId) const
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

ValCh<PaletteConfiguration::PaletteCellConfig> PaletteConfiguration::paletteCellConfig(const QString& cellId) const
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
