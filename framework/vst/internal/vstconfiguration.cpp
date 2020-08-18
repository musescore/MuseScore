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
#include "vstconfiguration.h"
#include "settings.h"

using namespace mu::vst;
using namespace mu::framework;

static const Settings::Key SEARCH_PATHS("vst", "search_path");

#ifdef Q_OS_MAC
const std::string VSTConfiguration::DEFAULT_PATHS = "/Library/Audio/Plug-Ins/VST3";
#elif Q_OS_WIN
const std::string VSTConfiguration::DEFAULT_PATHS = "C:\Program Files (x86)\Common Files\VST3";
#endif

void VSTConfiguration::init()
{
    settings()->addItem(SEARCH_PATHS, Val(VSTConfiguration::DEFAULT_PATHS));
}

std::string VSTConfiguration::searchPaths() const
{
    return settings()->value(SEARCH_PATHS).toString();
}
