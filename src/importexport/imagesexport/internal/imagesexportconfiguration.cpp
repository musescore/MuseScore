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
#include "imagesexportconfiguration.h"

#include "settings.h"

#include "engraving/dom/mscore.h"

using namespace muse;
using namespace mu;
using namespace mu::iex::imagesexport;

static const Settings::Key EXPORT_PDF_DPI_RESOLUTION_KEY("iex_imagesexport", "export/pdf/dpi");
static const Settings::Key EXPORT_PDF_USE_TRANSPARENCY_KEY("iex_imagesexport", "export/pdf/useTransparency");
static const Settings::Key EXPORT_PNG_DPI_RESOLUTION_KEY("iex_imagesexport", "export/png/resolution");
static const Settings::Key EXPORT_PNG_USE_TRANSPARENCY_KEY("iex_imagesexport", "export/png/useTransparency");
static const Settings::Key EXPORT_SVG_USE_TRANSPARENCY_KEY("iex_imagesexport", "export/svg/useTransparency");
static const Settings::Key EXPORT_SVG_ILLUSTRATOR_COMPAT("iex_imagesexport", "export/svg/illustratorCompat");

void ImagesExportConfiguration::init()
{
    settings()->setDefaultValue(EXPORT_PNG_DPI_RESOLUTION_KEY, Val(mu::engraving::DPI));
    settings()->setDefaultValue(EXPORT_PDF_DPI_RESOLUTION_KEY, Val(mu::engraving::DPI));
    settings()->setDefaultValue(EXPORT_PNG_USE_TRANSPARENCY_KEY, Val(false));
    settings()->setDefaultValue(EXPORT_SVG_ILLUSTRATOR_COMPAT, Val(false));
}

int ImagesExportConfiguration::exportPdfDpiResolution() const
{
    return settings()->value(EXPORT_PDF_DPI_RESOLUTION_KEY).toInt();
}

void ImagesExportConfiguration::setExportPdfDpiResolution(int dpi)
{
    settings()->setSharedValue(EXPORT_PDF_DPI_RESOLUTION_KEY, Val(dpi));
}

bool ImagesExportConfiguration::exportPdfWithTransparentBackground() const
{
    return settings()->value(EXPORT_PDF_USE_TRANSPARENCY_KEY).toBool();
}

void ImagesExportConfiguration::setExportPdfWithTransparentBackground(bool transparent)
{
    settings()->setSharedValue(EXPORT_PDF_USE_TRANSPARENCY_KEY, Val(transparent));
}

float ImagesExportConfiguration::exportPngDpiResolution() const
{
    if (m_customExportPngDpiOverride) {
        return m_customExportPngDpiOverride.value();
    }

    return settings()->value(EXPORT_PNG_DPI_RESOLUTION_KEY).toFloat();
}

void ImagesExportConfiguration::setExportPngDpiResolution(float dpi)
{
    settings()->setSharedValue(EXPORT_PNG_DPI_RESOLUTION_KEY, Val(dpi));
}

void ImagesExportConfiguration::setExportPngDpiResolutionOverride(std::optional<float> dpi)
{
    m_customExportPngDpiOverride = dpi;
}

bool ImagesExportConfiguration::exportPngWithTransparentBackground() const
{
    return settings()->value(EXPORT_PNG_USE_TRANSPARENCY_KEY).toBool();
}

void ImagesExportConfiguration::setExportPngWithTransparentBackground(bool transparent)
{
    settings()->setSharedValue(EXPORT_PNG_USE_TRANSPARENCY_KEY, Val(transparent));
}

bool ImagesExportConfiguration::exportSvgWithTransparentBackground() const
{
    return settings()->value(EXPORT_SVG_USE_TRANSPARENCY_KEY).toBool();
}

void ImagesExportConfiguration::setExportSvgWithTransparentBackground(bool transparent)
{
    settings()->setSharedValue(EXPORT_SVG_USE_TRANSPARENCY_KEY, Val(transparent));
}

bool ImagesExportConfiguration::exportSvgWithIllustratorCompat() const
{
    return settings()->value(EXPORT_SVG_ILLUSTRATOR_COMPAT).toBool();
}

void ImagesExportConfiguration::setExportSvgWithIllustratorCompat(bool compat)
{
    settings()->setSharedValue(EXPORT_SVG_ILLUSTRATOR_COMPAT, Val(compat));
}

int ImagesExportConfiguration::trimMarginPixelSize() const
{
    return m_trimMarginPixelSize ? m_trimMarginPixelSize.value() : -1;
}

void ImagesExportConfiguration::setTrimMarginPixelSize(std::optional<int> pixelSize)
{
    m_trimMarginPixelSize = pixelSize;
}
