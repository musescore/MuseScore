//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "synthesizercontroller.h"
#include "log.h"
#include "internal/audiosanitizer.h"

using namespace mu::audio::synth;

SynthesizerController::SynthesizerController(const ISynthesizersRegisterPtr& reg, const ISoundFontsProviderPtr& prov)
    : m_synthRegister(reg), m_soundFontProvider(prov)
{
    ONLY_AUDIO_WORKER_THREAD;
}

void SynthesizerController::init()
{
    ONLY_AUDIO_WORKER_THREAD;
    IF_ASSERT_FAILED(m_synthRegister) {
        return;
    }

    IF_ASSERT_FAILED(m_soundFontProvider) {
        return;
    }

    m_synthRegister->synthesizerAdded().onReceive(this, [](const ISynthesizerPtr& synth) {
        synth->init();
    });

    std::vector<ISynthesizerPtr> synthesizers = m_synthRegister->synthesizers();
    for (ISynthesizerPtr& synth : synthesizers) {
        synth->init();

        reloadSoundFonts(synth);
        auto notification = m_soundFontProvider->soundFontPathsForSynthChanged(synth->name());
        notification.onNotify(this, [this, synth]() {
            reloadSoundFonts(synth);
        });
    }
}

void SynthesizerController::reloadSoundFonts(std::shared_ptr<ISynthesizer> synth)
{
    ONLY_AUDIO_WORKER_THREAD;
    Ret ret = synth->removeSoundFonts();
    if (!ret) {
        LOGE() << "failed remove sound font, synth: " << synth->name();
    }

    std::vector<io::path> sfonts = m_soundFontProvider->soundFontPathsForSynth(synth->name());
    synth->addSoundFonts(sfonts);
}
