/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
