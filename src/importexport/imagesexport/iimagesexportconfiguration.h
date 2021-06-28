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
#ifndef MU_IMPORTEXPORT_IIMAGESEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_IIMAGESEXPORTCONFIGURATION_H

#include <string>
#include <optional>
#include "modularity/imoduleexport.h"

namespace mu::iex::imagesexport {
class IImagesExportConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IImagesExportConfiguration)

public:
    virtual ~IImagesExportConfiguration() = default;

    // Pdf
    virtual int exportPdfDpiResolution() const = 0;
    virtual void setExportPdfDpiResolution(int dpi) = 0;

    // Png
    virtual float exportPngDpiResolution() const = 0;

    //! NOTE Maybe set from command line
    virtual void setExportPngDpiResolution(std::optional<float> dpi) = 0;

    virtual bool exportPngWithTransparentBackground() const = 0;
    virtual void setExportPngWithTransparentBackground(bool transparent) = 0;

    virtual int trimMarginPixelSize() const = 0;
    virtual void setTrimMarginPixelSize(std::optional<int> pixelSize) = 0;
};
}

#endif // MU_IMPORTEXPORT_IIMAGESEXPORTCONFIGURATION_H
