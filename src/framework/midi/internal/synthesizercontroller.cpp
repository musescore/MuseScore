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

using namespace mu::midi;

SynthesizerController::SynthesizerController()
    : m_initilizer()
{
}

SynthesizerController::~SynthesizerController()
{
    if (m_initilizer.valid()) {
        m_initilizer.wait();
    }
}

void SynthesizerController::init()
{
    IF_ASSERT_FAILED(audioEngine()) {
        return;
    }

    if (audioEngine()->isInited()) {
        initSoundFonts(audioEngine()->sampleRate());
    } else {
        audioEngine()->initChanged().onReceive(this, [this](bool inited) {
            if (inited) {
                initSoundFonts(audioEngine()->sampleRate());
            }
        });
    }
}

void SynthesizerController::initSoundFonts(unsigned int sampleRate)
{
    auto task = [=]() {
        std::vector<std::shared_ptr<ISynthesizer> > synthesizers = synthRegister()->synthesizers();

        for (std::shared_ptr<ISynthesizer> synth : synthesizers) {
            synth->init(sampleRate);

            reloadSoundFonts(synth);
            auto notification = sfprovider()->soundFontPathsForSynthChanged(synth->name());
            notification.onNotify(this, [this, synth]() { reloadSoundFonts(synth); });
        }
    };
#ifndef Q_OS_WASM
    m_initilizer = std::async(task);
#else
    task();
#endif
}

void SynthesizerController::reloadSoundFonts(std::shared_ptr<ISynthesizer> synth)
{
    Ret ret = synth->removeSoundFonts();
    if (!ret) {
        LOGE() << "failed remove sound font, synth: " << synth->name();
    }

    std::vector<io::path> sfonts = sfprovider()->soundFontPathsForSynth(synth->name());
    synth->addSoundFonts(sfonts);
}
