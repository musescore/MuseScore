/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "midiremotestubmodule.h"

#include "modularity/ioc.h"

#include "midiremotestub.h"
#include "midiremoteconfigurationstub.h"

using namespace muse::midiremote;
using namespace muse::modularity;

static const std::string mname("midiremote_stub");

std::string MidiRemoteModule::moduleName() const
{
    return mname;
}

void MidiRemoteModule::registerExports()
{
    globalIoc()->registerExport<IMidiRemoteConfiguration>(mname, std::make_shared<MidiRemoteConfigurationStub>());
}

IContextSetup* MidiRemoteModule::newContext(const ContextPtr& ctx) const
{
    return new MidiRemoteContext(ctx);
}

void MidiRemoteContext::registerExports()
{
    ioc()->registerExport<IMidiRemote>(mname, std::make_shared<MidiRemoteStub>());
}
