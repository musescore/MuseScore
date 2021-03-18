//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "rpcaudioenginecontroller.h"

#include "internal/worker/audioengine.h"

using namespace mu::audio::rpc;

TargetName RpcAudioEngineController::target() const
{
    return TargetName::AudioEngine;
}

void RpcAudioEngineController::doBind()
{
    bindMethod("init", [](const Args&) {
        AudioEngine::instance()->init();
    });

    bindMethod("onDriverOpened", [](const Args& args) {
        int sampleRate = args.arg<int>(0);
        uint16_t readBufferSize = args.arg<uint16_t>(1);
        AudioEngine::instance()->onDriverOpened(sampleRate, readBufferSize);
    });

    bindMethod("setSampleRate", [](const Args& args) {
        int sampleRate = args.arg<int>(0);
        AudioEngine::instance()->setSampleRate(sampleRate);
    });

    bindMethod("setReadBufferSize", [](const Args& args) {
        uint16_t readBufferSize = args.arg<uint16_t>(0);
        AudioEngine::instance()->setReadBufferSize(readBufferSize);
    });
}
