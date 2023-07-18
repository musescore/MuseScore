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
#ifndef MU_PALETTE_PALETTECONFIGURATION_H
#define MU_PALETTE_PALETTECONFIGURATION_H

#include "../ipaletteconfiguration.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "ui/iuiconfiguration.h"

namespace mu::palette {
class PaletteConfiguration : public IPaletteConfiguration, public async::Asyncable
{
    INJECT(ui::IUiConfiguration, uiConfiguration)
    INJECT(framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    double paletteScaling() const override;
    void setPaletteScaling(double scale) override;

    double paletteSpatium() const override;

    ValCh<bool> isSinglePalette() const override;
    void setIsSinglePalette(bool isSingle) override;

    ValCh<bool> isSingleClickToOpenPalette() const override;
    void setIsSingleClickToOpenPalette(bool isSingleClick) override;

    QColor elementsBackgroundColor() const override;
    QColor elementsColor() const override;
    QColor gridColor() const override;
    QColor accentColor() const override;
    async::Notification colorsChanged() const override;

    io::path_t keySignaturesDirPath() const override;
    io::path_t timeSignaturesDirPath() const override;

    bool useFactorySettings() const override;
    bool enableExperimental() const override;

    ValCh<PaletteConfig> paletteConfig(const QString& paletteId) const override;
    void setPaletteConfig(const QString& paletteId, const PaletteConfig& config) override;

    ValCh<PaletteCellConfig> paletteCellConfig(const QString& cellId) const override;
    void setPaletteCellConfig(const QString& cellId, const PaletteCellConfig& config) override;

private:
    QColor themeColor(ui::ThemeStyleKey key) const;

    ValCh<bool> m_isSinglePalette;
    ValCh<bool> m_isSingleClickToOpenPalette;

    mutable QHash<QString, ValCh<PaletteConfig> > m_paletteConfigs;
    mutable QHash<QString, ValCh<PaletteCellConfig> > m_paletteCellsConfigs;
};
}

#endif // MU_PALETTE_PALETTECONFIGURATION_H
