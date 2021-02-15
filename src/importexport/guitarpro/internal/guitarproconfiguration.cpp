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
#include "guitarproconfiguration.h"

#include "settings.h"

#include "libmscore/mscore.h"

using namespace mu::framework;
using namespace mu::iex::guitarpro;

static const Settings::Key IMPORT_GUITARPRO_CHARSET_KEY("iex_guitarpro", "import/guitarpro/charset");

void GuitarProConfiguration::init()
{
    settings()->setDefaultValue(IMPORT_GUITARPRO_CHARSET_KEY, Val("UTF-8"));
}

std::string GuitarProConfiguration::importGuitarProCharset() const
{
    return settings()->value(IMPORT_GUITARPRO_CHARSET_KEY).toString();
}

void GuitarProConfiguration::setImportGuitarProCharset(const std::string& charset)
{
    settings()->setValue(IMPORT_GUITARPRO_CHARSET_KEY, Val(charset));
}
