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
#include "audioexportmodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationwritersregister.h"
#include "internal/mp3writer.h"
#include "internal/wavewriter.h"
#include "internal/oggwriter.h"
#include "internal/flacwriter.h"

using namespace mu::iex::audioexport;
using namespace mu::notation;

std::string AudioExportModule::moduleName() const
{
    return "iex_audioexport";
}

void AudioExportModule::resolveImports()
{
    auto writers = framework::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "wav" }, std::make_shared<WaveWriter>());
        writers->reg({ "mp3" }, std::make_shared<Mp3Writer>());
        writers->reg({ "ogg" }, std::make_shared<OggWriter>());
        writers->reg({ "flac" }, std::make_shared<FlacWriter>());
    }
}
