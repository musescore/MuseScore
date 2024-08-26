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
#ifndef MU_PALETTE_IPALETTECONFIGURATION_H
#define MU_PALETTE_IPALETTECONFIGURATION_H

#include <QColor>
#include <QSize>

#include "modularity/imoduleinterface.h"

#include "types/retval.h"
#include "io/path.h"

namespace mu::palette {
class IPaletteConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPaletteConfiguration)

public:
    virtual ~IPaletteConfiguration() = default;

    virtual double paletteSpatium() const = 0;

    virtual double paletteScaling() const = 0;
    virtual void setPaletteScaling(double scale) = 0;

    virtual muse::ValCh<bool> isSinglePalette() const = 0;
    virtual void setIsSinglePalette(bool isSingle) = 0;

    virtual muse::ValCh<bool> isSingleClickToOpenPalette() const = 0;
    virtual void setIsSingleClickToOpenPalette(bool isSingleClick) = 0;

    virtual muse::ValCh<bool> isPaletteDragEnabled() const = 0;
    virtual void setIsPaletteDragEnabled(bool enabled) = 0;

    virtual QColor elementsBackgroundColor() const = 0;
    virtual QColor elementsColor() const = 0;
    virtual QColor gridColor() const = 0;
    virtual QColor accentColor() const = 0;
    virtual muse::async::Notification colorsChanged() const = 0;

    virtual muse::io::path_t keySignaturesDirPath() const = 0;
    virtual muse::io::path_t timeSignaturesDirPath() const = 0;

    virtual bool useFactorySettings() const = 0;
    virtual bool enableExperimental() const = 0;

    struct PaletteConfig {
        QString name;
        QSize size;
        double elementOffset = 0;
        double scale = 0;
        bool showGrid = false;
    };

    struct PaletteCellConfig {
        QString name;
        double xOffset = 0;
        double yOffset = 0;
        double scale = 0;
        bool drawStaff = false;
    };

    virtual muse::ValCh<PaletteConfig> paletteConfig(const QString& paletteId) const = 0;
    virtual void setPaletteConfig(const QString& paletteId, const PaletteConfig& config) = 0;

    virtual muse::ValCh<PaletteCellConfig> paletteCellConfig(const QString& cellId) const = 0;
    virtual void setPaletteCellConfig(const QString& cellId, const PaletteCellConfig& config) = 0;
};
}

#endif // MU_PALETTE_IPALETTECONFIGURATION_H
