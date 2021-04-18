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

static const std::string module_name("iex_musicxml");

static const Settings::Key MUSICXML_IMPORT_BREAKS_KEY(module_name, "import/musicXML/importBreaks");
static const Settings::Key MUSICXML_IMPORT_LAYOUT_KEY(module_name, "import/musicXML/importLayout");
static const Settings::Key MUSICXML_EXPORT_LAYOUT_KEY(module_name, "export/musicXML/exportLayout");
static const Settings::Key MUSICXML_EXPORT_BREAKS_TYPE_KEY(module_name, "export/musicXML/exportBreaks");
static const Settings::Key MIGRATION_APPLY_EDWIN_FOR_XML(module_name, "import/compatibility/apply_edwin_for_xml");
static const Settings::Key MIGRATION_NOT_ASK_AGAING_KEY(module_name, "import/compatibility/do_not_ask_me_again");
static const Settings::Key STYLE_FILE_IMPORT_PATH_KEY(module_name, "import/style/styleFile");

void MusicXmlConfiguration::init()
{
    settings()->setDefaultValue(MUSICXML_IMPORT_BREAKS_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(static_cast<int>(MusicxmlExportBreaksType::All)));
    settings()->setDefaultValue(MIGRATION_NOT_ASK_AGAING_KEY, Val(false));
}

bool MusicXmlConfiguration::musicxmlImportBreaks() const
{
    return settings()->value(MUSICXML_IMPORT_BREAKS_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlImportBreaks(bool value)
{
    settings()->setValue(MUSICXML_IMPORT_BREAKS_KEY, Val(value));
}

bool MusicXmlConfiguration::musicxmlImportLayout() const
{
    return settings()->value(MUSICXML_IMPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlImportLayout(bool value)
{
    settings()->setValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(value));
}

bool MusicXmlConfiguration::musicxmlExportLayout() const
{
    return settings()->value(MUSICXML_EXPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlExportLayout(bool value)
{
    settings()->setValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(value));
}

MusicXmlConfiguration::MusicxmlExportBreaksType MusicXmlConfiguration::musicxmlExportBreaksType() const
{
    return static_cast<MusicxmlExportBreaksType>(settings()->value(MUSICXML_EXPORT_BREAKS_TYPE_KEY).toInt());
}

bool MusicXmlConfiguration::needUseDefaultFont() const
{
    return settings()->value(MIGRATION_APPLY_EDWIN_FOR_XML).toBool();
}

void MusicXmlConfiguration::setNeedUseDefaultFont(bool value)
{
    settings()->setValue(MIGRATION_APPLY_EDWIN_FOR_XML, Val(value));
}

bool MusicXmlConfiguration::needAskAboutApplyingNewStyle() const
{
    return !settings()->value(MIGRATION_NOT_ASK_AGAING_KEY).toBool();
}

void MusicXmlConfiguration::setNeedAskAboutApplyingNewStyle(bool value)
{
    settings()->setValue(MIGRATION_NOT_ASK_AGAING_KEY, Val(!value));
}

mu::io::path MusicXmlConfiguration::styleFileImportPath() const
{
    return settings()->value(STYLE_FILE_IMPORT_PATH_KEY).toString();
}

void MusicXmlConfiguration::setStyleFileImportPath(const io::path& path)
{
    settings()->setValue(STYLE_FILE_IMPORT_PATH_KEY, Val(path.toStdString()));
}
