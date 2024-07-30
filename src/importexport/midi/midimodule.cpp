/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "midimodule.h"

#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/notationmidireader.h"
#include "project/inotationwritersregister.h"
#include "internal/notationmidiwriter.h"

#include "internal/midiconfiguration.h"

#include "log.h"

using namespace muse::modularity;
using namespace mu::iex::midi;
using namespace mu::project;

std::string MidiModule::moduleName() const
{
    return "iex_midi";
}

void MidiModule::registerExports()
{
    m_configuration = std::make_shared<MidiConfiguration>();

    ioc()->registerExport<IMidiImportExportConfiguration>(moduleName(), m_configuration);
}

void MidiModule::resolveImports()
{
    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiReader>());
    }

    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiWriter>(iocContext()));
    }
}

void MidiModule::onInit(const muse::IApplication::RunMode& mode)
{
    if (mode == muse::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}
