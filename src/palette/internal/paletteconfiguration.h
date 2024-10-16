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
#ifndef MU_PALETTE_PALETTECONFIGURATION_H
#define MU_PALETTE_PALETTECONFIGURATION_H

#include "../ipaletteconfiguration.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "ui/iuiconfiguration.h"

namespace mu::palette {
class PaletteConfiguration : public IPaletteConfiguration, public muse::async::Asyncable
{
    INJECT(muse::ui::IUiConfiguration, uiConfiguration)
    INJECT(muse::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    double paletteScaling() const override;
    void setPaletteScaling(double scale) override;

    double paletteSpatium() const override;

    muse::ValCh<bool> isSinglePalette() const override;
    void setIsSinglePalette(bool isSingle) override;

    muse::ValCh<bool> isSingleClickToOpenPalette() const override;
    void setIsSingleClickToOpenPalette(bool isSingleClick) override;

    muse::ValCh<bool> isPaletteDragEnabled() const override;
    void setIsPaletteDragEnabled(bool enabled) override;

    QColor elementsBackgroundColor() const override;
    QColor elementsColor() const override;
    QColor gridColor() const override;
    QColor accentColor() const override;
    muse::async::Notification colorsChanged() const override;

    muse::io::path_t keySignaturesDirPath() const override;
    muse::io::path_t timeSignaturesDirPath() const override;

    bool useFactorySettings() const override;
    bool enableExperimental() const override;

    muse::ValCh<PaletteConfig> paletteConfig(const QString& paletteId) const override;
    void setPaletteConfig(const QString& paletteId, const PaletteConfig& config) override;

    muse::ValCh<PaletteCellConfig> paletteCellConfig(const QString& cellId) const override;
    void setPaletteCellConfig(const QString& cellId, const PaletteCellConfig& config) override;

private:
    QColor themeColor(muse::ui::ThemeStyleKey key) const;

    muse::ValCh<bool> m_isSinglePalette;
    muse::ValCh<bool> m_isSingleClickToOpenPalette;
    muse::ValCh<bool> m_isPaletteDragEnabled;

    mutable QHash<QString, muse::ValCh<PaletteConfig> > m_paletteConfigs;
    mutable QHash<QString, muse::ValCh<PaletteCellConfig> > m_paletteCellsConfigs;
};
}

#endif // MU_PALETTE_PALETTECONFIGURATION_H
