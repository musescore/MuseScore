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
#include "audioexportmodule.h"

#include "modularity/ioc.h"

#include "project/inotationwritersregister.h"
#include "internal/mp3writer.h"
#include "internal/wavewriter.h"
#include "internal/oggwriter.h"
#include "internal/flacwriter.h"

#include "internal/audioexportconfiguration.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace mu::iex::audioexport;
using namespace mu::project;

std::string AudioExportModule::moduleName() const
{
    return "iex_audioexport";
}

void AudioExportModule::registerExports()
{
    m_configuration = std::make_shared<AudioExportConfiguration>();

    ioc()->registerExport<AudioExportConfiguration>(moduleName(), m_configuration);
}

void AudioExportModule::resolveImports()
{
    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "wav" }, std::make_shared<WaveWriter>(iocContext()));
        writers->reg({ "mp3" }, std::make_shared<Mp3Writer>(iocContext()));
        writers->reg({ "ogg" }, std::make_shared<OggWriter>(iocContext()));
        writers->reg({ "flac" }, std::make_shared<FlacWriter>(iocContext()));
    }
}

void AudioExportModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}
