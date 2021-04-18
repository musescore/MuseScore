//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#include "vstconfiguration.h"
#include "settings.h"

using namespace mu::vst;
using namespace mu::framework;

static const Settings::Key CUSTOM_SEARCH_PATH_KEY = Settings::Key("vst", "custom_search_path");

mu::io::path VstConfiguration::customSearchPath() const
{
    return mu::io::path(settings()->value(CUSTOM_SEARCH_PATH_KEY).toString());
}
