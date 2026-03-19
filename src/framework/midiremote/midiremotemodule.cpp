/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "midiremotemodule.h"

#include "midiremote/internal/midiremoteconfiguration.h"
#include "midiremote/internal/midiremote.h"

#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

using namespace muse::midiremote;

std::string MidiRemoteModule::moduleName() const
{
    return "midiremote";
}

void MidiRemoteModule::registerExports()
{
    m_configuration = std::make_shared<MidiRemoteConfiguration>(iocContext());
    m_midiRemote = std::make_shared<MidiRemote>(iocContext());

    ioc()->registerExport<IMidiRemoteConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IMidiRemote>(moduleName(), m_midiRemote);

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("midiMappingUserAppDataPath", m_configuration->midiMappingUserAppDataPath());
    }
#endif
}

void MidiRemoteModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_midiRemote->init();
}
