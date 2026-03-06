/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

 #include "enginesetup.h"

#include "internal/audioengineconfiguration.h"
#include "internal/synthesizers/synthresolver.h"
#include "internal/synthesizers/fluidsynth/fluidresolver.h"
#include "internal/synthesizers/soundfontrepository.h"
#include "internal/fx/fxresolver.h"
#include "internal/fx/musefxresolver.h"

#include "internal/audioengine.h"
#include "internal/engineplayback.h"

using namespace muse::audio::engine;

static const std::string mname("audio_engine");

// Global
std::string EngineGlobalSetup::moduleName() const
{
    return mname;
}

void EngineGlobalSetup::registerExports()
{
    //! NOTE Services registration
    //! At the moment, it is better to create services
    //! and register them in the IOC in the main thread,
    //! because the IOC thread is not safe.
    //! Therefore, we will need to make own IOC instance for the engine.

    m_configuration = std::make_shared<AudioEngineConfiguration>();
    m_synthResolver = std::make_shared<synth::SynthResolver>();
    m_fxResolver = std::make_shared<fx::FxResolver>();
    m_soundFontRepository = std::make_shared<synth::SoundFontRepository>();

    globalIoc()->registerExport<IAudioEngineConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<synth::ISynthResolver>(mname, m_synthResolver);
    globalIoc()->registerExport<fx::IFxResolver>(mname, m_fxResolver);
    globalIoc()->registerExport<synth::ISoundFontRepository>(mname, m_soundFontRepository);
}

void EngineGlobalSetup::resolveImports()
{
    m_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<fx::MuseFxResolver>());
    m_synthResolver->registerResolver(AudioSourceType::Fluid, std::make_shared<synth::FluidResolver>());
}

void EngineGlobalSetup::onDeinit()
{
    //! MAIN THREAD
    globalIoc()->unregister<IAudioEngineConfiguration>(mname);
    globalIoc()->unregister<synth::ISynthResolver>(mname);
    globalIoc()->unregister<synth::ISoundFontRepository>(mname);
    globalIoc()->unregister<fx::IFxResolver>(mname);
}

// Context
void EngineContextSetup::registerExports()
{
    m_audioEngine = std::make_shared<AudioEngine>();
    m_playback = std::make_shared<EnginePlayback>(iocContext());

    ioc()->registerExport<IAudioEngine>(mname, m_audioEngine);
    ioc()->registerExport<IEnginePlayback>(mname, m_playback);
}

void EngineContextSetup::onDeinit()
{
    ioc()->unregister<IAudioEngine>(mname);
    ioc()->unregister<IEnginePlayback>(mname);
}
