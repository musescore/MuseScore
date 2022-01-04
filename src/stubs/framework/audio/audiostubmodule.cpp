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
#include "audiostubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "audioconfigurationstub.h"
#include "audiodriverstub.h"
#include "synthesizersregisterstub.h"
#include "soundfontsproviderstub.h"

using namespace mu::modularity;
using namespace mu::audio;

static void audio_init_qrc()
{
    Q_INIT_RESOURCE(audio);
}

std::string AudioStubModule::moduleName() const
{
    return "audio_engine_stub";
}

void AudioStubModule::registerExports()
{
    ioc()->registerExport<IAudioConfiguration>(moduleName(), new AudioConfigurationStub());
    ioc()->registerExport<IAudioDriver>(moduleName(), new AudioDriverStub());

    ioc()->registerExport<synth::ISynthesizersRegister>(moduleName(), new synth::SynthesizersRegisterStub());
    ioc()->registerExport<synth::ISoundFontsProvider>(moduleName(), new synth::SoundFontsProviderStub());
}

void AudioStubModule::registerResources()
{
    audio_init_qrc();
}

void AudioStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(audio_QML_IMPORT);
}
