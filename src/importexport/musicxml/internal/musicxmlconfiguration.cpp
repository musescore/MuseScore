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
#include "musicxmlconfiguration.h"

#include "settings.h"

using namespace mu::framework;
using namespace mu::iex::musicxml;

static const Settings::Key MUSICXML_IMPORT_BREAKS_KEY("iex_musicxml", "import/musicXML/importBreaks");
static const Settings::Key MUSICXML_IMPORT_LAYOUT_KEY("iex_musicxml", "import/musicXML/importLayout");
static const Settings::Key MUSICXML_EXPORT_LAYOUT_KEY("iex_musicxml", "export/musicXML/exportLayout");
static const Settings::Key MUSICXML_EXPORT_BREAKS_TYPE_KEY("iex_musicxml", "export/musicXML/exportBreaks");
static const Settings::Key MIGRATION_APPLY_EDWIN_FOR_XML("iex_musicxml", "import/compatibility/apply_edwin_for_xml");

void MusicXmlConfiguration::init()
{
    settings()->setDefaultValue(MUSICXML_IMPORT_BREAKS_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(static_cast<int>(MusicxmlExportBreaksType::All)));
}

bool MusicXmlConfiguration::musicxmlImportBreaks() const
{
    return settings()->value(MUSICXML_IMPORT_BREAKS_KEY).toBool();
}

bool MusicXmlConfiguration::musicxmlImportLayout() const
{
    return settings()->value(MUSICXML_IMPORT_LAYOUT_KEY).toBool();
}

bool MusicXmlConfiguration::musicxmlExportLayout() const
{
    return settings()->value(MUSICXML_EXPORT_LAYOUT_KEY).toBool();
}

MusicXmlConfiguration::MusicxmlExportBreaksType MusicXmlConfiguration::musicxmlExportBreaksType() const
{
    return static_cast<MusicxmlExportBreaksType>(settings()->value(MUSICXML_EXPORT_BREAKS_TYPE_KEY).toInt());
}

bool MusicXmlConfiguration::needUseDefaultFont() const
{
    return settings()->value(MIGRATION_APPLY_EDWIN_FOR_XML).toBool();
}
