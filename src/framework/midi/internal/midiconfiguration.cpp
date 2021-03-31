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
#include "midiconfiguration.h"

#include "settings.h"

using namespace mu::midi;
using namespace mu::framework;

static const std::string module_name("midi");

static const Settings::Key USE_REMOTE_CONTROL_KEY(module_name, "io/midi/useRemoteControl");

void MidiConfiguration::init()
{
    settings()->setDefaultValue(USE_REMOTE_CONTROL_KEY, Val(true));
}

bool MidiConfiguration::useRemoteControl() const
{
    return settings()->value(USE_REMOTE_CONTROL_KEY).toBool();
}

void MidiConfiguration::setUseRemoteControl(bool value)
{
    return settings()->setValue(USE_REMOTE_CONTROL_KEY, Val(value));
}
