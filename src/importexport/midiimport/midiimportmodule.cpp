/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "midiimportmodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/notationmidireader.h"
#include "notation/inotationwritersregister.h"
#include "internal/notationmidiwriter.h"

#include "internal/midiimportconfiguration.h"

using namespace mu::iex::midiimport;
using namespace mu::notation;

static std::shared_ptr<MidiImportConfiguration> s_configuration = std::make_shared<MidiImportConfiguration>();

std::string MidiImportModule::moduleName() const
{
    return "iex_midiimport";
}

void MidiImportModule::registerExports()
{
    framework::ioc()->registerExport<IMidiImportConfiguration>(moduleName(), s_configuration);
}

void MidiImportModule::resolveImports()
{
    auto readers = framework::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiReader>());
    }

    auto writers = framework::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiWriter>());
    }
}

void MidiImportModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
}
