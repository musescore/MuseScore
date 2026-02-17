/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "audioconfigurationstub.h"
#include "audiodrivercontrollerstub.h"
#include "synthresolverstub.h"
#include "fxresolverstub.h"
#include "soundfontcontrollerstub.h"

using namespace muse::modularity;
using namespace muse::audio;

std::string AudioModule::moduleName() const
{
    return "audio_stub";
}

void AudioModule::registerExports()
{
    globalIoc()->registerExport<IAudioConfiguration>(moduleName(), new AudioConfigurationStub());
    globalIoc()->registerExport<IAudioDriverController>(moduleName(), new AudioDriverControllerStub());

    globalIoc()->registerExport<synth::ISynthResolver>(moduleName(), new synth::SynthResolverStub());
    globalIoc()->registerExport<fx::IFxResolver>(moduleName(), new fx::FxResolverStub());

    globalIoc()->registerExport<ISoundFontController>(moduleName(), new SoundFontControllerStub());
}
