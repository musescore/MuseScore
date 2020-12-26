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
#include "importexportconfiguration.h"

#include "settings.h"

#include "libmscore/mscore.h"

using namespace mu::framework;
using namespace mu::importexport;

static const std::string module_name("importexport");

static const Settings::Key SHORTEST_NOTE_KEY(module_name, "io/midi/shortestNote");
static const Settings::Key IMPORT_OVERTUNE_CHARSET_KEY(module_name, "import/overture/charset");
static const Settings::Key IMPORT_GUITARPRO_CHARSET_KEY(module_name, "import/guitarpro/charset");
static const Settings::Key MUSICXML_IMPORT_BREAKS_KEY(module_name, "import/musicXML/importBreaks");
static const Settings::Key MUSICXML_IMPORT_LAYOUT_KEY(module_name, "import/musicXML/importLayout");
static const Settings::Key MUSICXML_EXPORT_LAYOUT_KEY(module_name, "export/musicXML/exportLayout");
static const Settings::Key MUSICXML_EXPORT_BREAKS_TYPE_KEY(module_name, "export/musicXML/exportBreaks");
static const Settings::Key EXPORT_PDF_DPI_RESOLUTION_KEY(module_name, "export/pdf/dpi");
static const Settings::Key EXPORT_PNG_DPI_RESOLUTION_KEY(module_name, "export/png/resolution");
static const Settings::Key EXPORT_PNG_USE_TRASNPARENCY_KEY(module_name, "export/png/useTransparency");

void ImportexportConfiguration::init()
{
    settings()->setDefaultValue(SHORTEST_NOTE_KEY, Val(Ms::MScore::division / 4));

    settings()->setDefaultValue(IMPORT_OVERTUNE_CHARSET_KEY, Val("GBK"));
    settings()->setDefaultValue(IMPORT_GUITARPRO_CHARSET_KEY, Val("UTF-8"));

    settings()->setDefaultValue(MUSICXML_IMPORT_BREAKS_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(static_cast<int>(MusicxmlExportBreaksType::All)));

    settings()->setDefaultValue(EXPORT_PNG_DPI_RESOLUTION_KEY, Val(Ms::DPI));
    settings()->setDefaultValue(EXPORT_PNG_USE_TRASNPARENCY_KEY, Val(true));
    settings()->setDefaultValue(EXPORT_PDF_DPI_RESOLUTION_KEY, Val(Ms::DPI));
}

int ImportexportConfiguration::midiShortestNote() const
{
    return settings()->value(SHORTEST_NOTE_KEY).toInt();
}

std::string ImportexportConfiguration::importOvertuneCharset() const
{
    return settings()->value(IMPORT_OVERTUNE_CHARSET_KEY).toString();
}

std::string ImportexportConfiguration::importGuitarProCharset() const
{
    return settings()->value(IMPORT_GUITARPRO_CHARSET_KEY).toString();
}

bool ImportexportConfiguration::musicxmlImportBreaks() const
{
    return settings()->value(MUSICXML_IMPORT_BREAKS_KEY).toBool();
}

bool ImportexportConfiguration::musicxmlImportLayout() const
{
    return settings()->value(MUSICXML_IMPORT_LAYOUT_KEY).toBool();
}

bool ImportexportConfiguration::musicxmlExportLayout() const
{
    return settings()->value(MUSICXML_EXPORT_LAYOUT_KEY).toBool();
}

ImportexportConfiguration::MusicxmlExportBreaksType ImportexportConfiguration::musicxmlExportBreaksType() const
{
    return static_cast<MusicxmlExportBreaksType>(settings()->value(MUSICXML_EXPORT_BREAKS_TYPE_KEY).toInt());
}

int ImportexportConfiguration::exportPdfDpiResolution() const
{
    return settings()->value(EXPORT_PDF_DPI_RESOLUTION_KEY).toInt();
}

void ImportexportConfiguration::setExportPngDpiResolution(std::optional<float> dpi)
{
    m_customExportPngDpi = dpi;
}

float ImportexportConfiguration::exportPngDpiResolution() const
{
    if (m_customExportPngDpi) {
        return m_customExportPngDpi.value();
    }

    return settings()->value(EXPORT_PNG_DPI_RESOLUTION_KEY).toFloat();
}

bool ImportexportConfiguration::exportPngWithTransparentBackground() const
{
    return settings()->value(EXPORT_PNG_USE_TRASNPARENCY_KEY).toBool();
}
