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
#pragma once

#include "../iimagesexportconfiguration.h"

namespace mu::iex::imagesexport {
class ImagesExportConfiguration : public IImagesExportConfiguration
{
public:
    void init();

    int exportPdfDpiResolution() const override;
    void setExportPdfDpiResolution(int dpi) override;

    bool exportPdfWithTransparentBackground() const override;
    void setExportPdfWithTransparentBackground(bool transparent) override;

    bool exportPdfWithGrayscale() const override;
    void setExportPdfWithGrayscale(bool grayscale) override;

    bool exportPdfWithEmbeddedMetadata() const override;
    void setExportPdfWithEmbeddedMetadata(bool embedMetadata) override;

    float exportPngDpiResolution() const override;
    void setExportPngDpiResolution(float dpi) override;
    void setExportPngDpiResolutionOverride(std::optional<float> dpi) override;

    bool exportPngWithTransparentBackground() const override;
    void setExportPngWithTransparentBackground(bool transparent) override;

    bool exportPngWithGrayscale() const override;
    void setExportPngWithGrayscale(bool grayscale) override;

    bool exportSvgWithTransparentBackground() const override;
    void setExportSvgWithTransparentBackground(bool transparent) override;
    bool exportSvgWithIllustratorCompat() const override;
    void setExportSvgWithIllustratorCompat(bool compat) override;

    int trimMarginPixelSize() const override;
    void setTrimMarginPixelSize(std::optional<int> pixelSize) override;

private:
    std::optional<int> m_trimMarginPixelSize;
    std::optional<float> m_customExportPngDpiOverride;
};
}
