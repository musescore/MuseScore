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

static const std::string mname("audio_stub");

std::string AudioModule::moduleName() const
{
    return mname;
}

void AudioModule::registerExports()
{
    globalIoc()->registerExport<IAudioConfiguration>(mname, new AudioConfigurationStub());
    globalIoc()->registerExport<synth::ISynthResolver>(mname, new synth::SynthResolverStub());
    globalIoc()->registerExport<fx::IFxResolver>(mname, new fx::FxResolverStub());
    globalIoc()->registerExport<IAudioDriverController>(mname, new AudioDriverControllerStub());
    globalIoc()->registerExport<ISoundFontController>(mname, new SoundFontControllerStub());
}

IContextSetup* AudioModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AudioContext(ctx);
}

void AudioContext::registerExports()
{
}
