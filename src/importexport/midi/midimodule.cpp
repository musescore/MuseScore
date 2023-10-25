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
#include "midimodule.h"

#include "modularity/ioc.h"

#ifdef MUE_ENABLE_MIDI_IMPORTEXPORT
#include "project/inotationreadersregister.h"
#include "project/inotationwritersregister.h"
#include "internal/notationmidireader.h"
#include "internal/notationmidiwriter.h"
#include "internal/midiconfiguration.h"
using namespace mu::project;
#endif
#include "internal/midirender/midirender.h"

#include "log.h"

namespace mu::iex::midi
{
    std::string MidiModule::moduleName() const
    {
        return "iex_midi";
    }

    void MidiModule::registerExports()
    {
#ifdef MUE_ENABLE_MIDI_IMPORTEXPORT
        m_configuration = std::make_shared<MidiConfiguration>();

        modularity::ioc()->registerExport<IMidiImportExportConfiguration>(moduleName(), m_configuration);
//        modularity::ioc()->registerExport<IMidiRenderStrategy>(moduleName());
#endif
        modularity::ioc()->registerExport<IMidiRender>(moduleName(), new MidiRender());
    }

#ifdef MUE_ENABLE_MIDI_IMPORTEXPORT
    void MidiModule::resolveImports()
    {
        auto readers = modularity::ioc()->resolve<INotationReadersRegister>(moduleName());
        if (readers) {
            readers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiReader>());
        }

        auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
        if (writers) {
            writers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiWriter>());
        }
    }

    void MidiModule::onInit(const framework::IApplication::RunMode& mode)
    {
        if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
            return;
        }

        m_configuration->init();
    }
#endif
}

