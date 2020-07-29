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
#include "synthesizersetup.h"

#include "log.h"

using namespace mu::midi;

void SynthesizerSetup::setup()
{
    IF_ASSERT_FAILED(audioEngine()) {
        return;
    }

    auto init = [this](float sampleRate) {
                    synth()->init(sampleRate);

                    io::path sfPath = globalConfiguration()->dataPath() + "/sound/GeneralUser GS v1.471.sf2";
                    synth()->loadSF(sfPath);
                };

    if (audioEngine()->isInited()) {
        init(audioEngine()->sampleRate());
    } else {
        audioEngine()->initChanged().onReceive(this, [this, init](bool inited) {
            if (inited) {
                init(audioEngine()->sampleRate());
            }
        });
    }
}
