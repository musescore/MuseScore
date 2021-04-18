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
#include "oveconfiguration.h"

#include "settings.h"

using namespace mu::framework;
using namespace mu::iex::ove;

static const Settings::Key IMPORT_OVERTUNE_CHARSET_KEY("iex_ove", "import/overture/charset");

void OveConfiguration::init()
{
    settings()->setDefaultValue(IMPORT_OVERTUNE_CHARSET_KEY, Val("GBK"));
}

std::string OveConfiguration::importOvertuneCharset() const
{
    return settings()->value(IMPORT_OVERTUNE_CHARSET_KEY).toString();
}

void OveConfiguration::setImportOvertuneCharset(const std::string& charset)
{
    settings()->setValue(IMPORT_OVERTUNE_CHARSET_KEY, Val(charset));
}
