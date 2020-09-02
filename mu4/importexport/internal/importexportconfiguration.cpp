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

static const std::string module_name("importexport");

using namespace mu::framework;
using namespace mu::importexport;

int ImportexportConfiguration::midiShortestNote() const
{
    return settings()->value(Settings::Key(module_name, "io/midi/shortestNote")).toInt();
}

std::string ImportexportConfiguration::importOvertuneCharset() const
{
    return settings()->value(Settings::Key(module_name, "import/overture/charset")).toString();
}

std::string ImportexportConfiguration::importGuitarProCharset() const
{
    return settings()->value(Settings::Key(module_name, "import/guitarpro/charset")).toString();
}

bool ImportexportConfiguration::musicxmlImportBreaks() const
{
    return settings()->value(Settings::Key(module_name, "import/musicXML/importBreaks")).toBool();
}

bool ImportexportConfiguration::musicxmlImportLayout() const
{
    return settings()->value(Settings::Key(module_name, "import/musicXML/importLayout")).toBool();
}
