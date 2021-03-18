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
#ifndef MU_PALETTE_PALETTECONFIGURATION_H
#define MU_PALETTE_PALETTECONFIGURATION_H

#include "../ipaletteconfiguration.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "async/asyncable.h"

namespace mu::palette {
class PaletteConfiguration : public IPaletteConfiguration, public async::Asyncable
{
    INJECT(palette, ui::IUiConfiguration, uiConfiguration)
    INJECT(palette, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(palette, notation::INotationConfiguration, notationConfiguration)

public:
    void init();

    double paletteScaling() const override;
    bool isSinglePalette() const override;

    QColor elementsBackgroundColor() const override;
    QColor elementsColor() const override;
    QColor gridColor() const override;
    QColor accentColor() const override;
    async::Notification colorsChanged() const override;

    bool useNotationForegroundColor() const override;
    void setUseNotationForegroundColor(bool value) override;

    io::path keySignaturesDirPath() const override;
    io::path timeSignaturesDirPath() const override;

    bool useFactorySettings() const override;
    bool enableExperimental() const override;

    ValCh<PaletteConfig> paletteConfig(const QString& paletteId) const override;
    void setPaletteConfig(const QString& paletteId, const PaletteConfig& config) override;

    ValCh<PaletteCellConfig> paletteCellConfig(const QString& cellId) const override;
    void setPaletteCellConfig(const QString& cellId, const PaletteCellConfig& config) override;

private:
    QColor themeColor(ui::ThemeStyleKey key) const;

    mutable QHash<QString, ValCh<PaletteConfig> > m_paletteConfigs;
    mutable QHash<QString, ValCh<PaletteCellConfig> > m_paletteCellsConfigs;
    async::Notification m_colorsChanged;
};
}

#endif // MU_PALETTE_PALETTECONFIGURATION_H
