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
#include "synthresolverstub.h"
#include "fxresolverstub.h"
#include "soundfontrepositorystub.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;

static void audio_init_qrc()
{
    Q_INIT_RESOURCE(audio);
}

std::string AudioModule::moduleName() const
{
    return "audio_engine_stub";
}

void AudioModule::registerExports()
{
    ioc()->registerExport<IAudioConfiguration>(moduleName(), new AudioConfigurationStub());
    ioc()->registerExport<IAudioDriver>(moduleName(), new AudioDriverStub());

    ioc()->registerExport<synth::ISynthResolver>(moduleName(), new synth::SynthResolverStub());
    ioc()->registerExport<fx::IFxResolver>(moduleName(), new fx::FxResolverStub());

    ioc()->registerExport<ISoundFontRepository>(moduleName(), new SoundFontRepositoryStub());
}

void AudioModule::registerResources()
{
    audio_init_qrc();
}

void AudioModule::registerUiTypes()
{
}
