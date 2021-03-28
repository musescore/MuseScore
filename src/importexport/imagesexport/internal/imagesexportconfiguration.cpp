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
#include "imagesexportconfiguration.h"

#include "settings.h"

#include "libmscore/mscore.h"

using namespace mu::framework;
using namespace mu::iex::imagesexport;

static const Settings::Key EXPORT_PDF_DPI_RESOLUTION_KEY("iex_imagesexport", "export/pdf/dpi");
static const Settings::Key EXPORT_PNG_DPI_RESOLUTION_KEY("iex_imagesexport", "export/png/resolution");
static const Settings::Key EXPORT_PNG_USE_TRASNPARENCY_KEY("iex_imagesexport", "export/png/useTransparency");

void ImagesExportConfiguration::init()
{
    settings()->setDefaultValue(EXPORT_PDF_DPI_RESOLUTION_KEY, Val(Ms::DPI));
    settings()->setInfo(EXPORT_PDF_DPI_RESOLUTION_KEY, std::make_shared<NumberSpinnerInfo>(
                            QT_TRANSLATE_NOOP("settings", "Resolution:"),
                            72, 2400, 1, 0,
                            QT_TRANSLATE_NOOP("settings", "dpi"))
                        );

    // Might be used for testing in Export Dialog
    //std::vector<std::pair<QVariant, const char*>> options {
    //    { 100, "100" },
    //    { 200, "200" },
    //    { 300, "300" },
    //    { 400, "400" }
    //};
    //
    //settings()->setInfo(EXPORT_PDF_DPI_RESOLUTION_KEY, std::make_shared<PickerInfo>(PickerInfo::RadioButtonGroup, "Resolution:", options));

    settings()->setDefaultValue(EXPORT_PNG_DPI_RESOLUTION_KEY, Val(Ms::DPI));
    settings()->setInfo(EXPORT_PNG_DPI_RESOLUTION_KEY, std::make_shared<NumberSpinnerInfo>(
                            QT_TRANSLATE_NOOP("settings", "Resolution:"),
                            32, 5000, 1, 0,
                            QT_TRANSLATE_NOOP("settings", "dpi"))
                        );

    settings()->setDefaultValue(EXPORT_PNG_USE_TRASNPARENCY_KEY, Val(true));
    settings()->setInfo(EXPORT_PNG_USE_TRASNPARENCY_KEY, std::make_shared<SettingsInfo>(
                            QT_TRANSLATE_NOOP("settings", "Transparent background"))
                        );
}

int ImagesExportConfiguration::exportPdfDpiResolution() const
{
    return settings()->value(EXPORT_PDF_DPI_RESOLUTION_KEY).toInt();
}

void ImagesExportConfiguration::setExportPdfDpiResolution(int dpi)
{
    settings()->setValue(EXPORT_PDF_DPI_RESOLUTION_KEY, Val(dpi));
}

void ImagesExportConfiguration::setExportPngDpiResolution(std::optional<float> dpi)
{
    m_customExportPngDpi = dpi;
}

float ImagesExportConfiguration::exportPngDpiResolution() const
{
    if (m_customExportPngDpi) {
        return m_customExportPngDpi.value();
    }

    return settings()->value(EXPORT_PNG_DPI_RESOLUTION_KEY).toFloat();
}

bool ImagesExportConfiguration::exportPngWithTransparentBackground() const
{
    return settings()->value(EXPORT_PNG_USE_TRASNPARENCY_KEY).toBool();
}

void ImagesExportConfiguration::setExportPngWithTransparentBackground(bool transparent)
{
    settings()->setValue(EXPORT_PNG_USE_TRASNPARENCY_KEY, Val(transparent));
}
