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
#include "audioconfigurationstub.h"

using namespace mu::audio;
using namespace mu;

unsigned int AudioConfigurationStub::driverBufferSize() const
{
    return 0;
}

std::vector<io::path> AudioConfigurationStub::soundFontPaths() const
{
    return {};
}

const synth::SynthesizerState& AudioConfigurationStub::synthesizerState() const
{
    static const synth::SynthesizerState state;
    return state;
}

Ret AudioConfigurationStub::saveSynthesizerState(const synth::SynthesizerState&)
{
    return make_ret(Ret::Code::NotSupported);
}

async::Notification AudioConfigurationStub::synthesizerStateChanged() const
{
    return async::Notification();
}

async::Notification AudioConfigurationStub::synthesizerStateGroupChanged(const std::string&) const
{
    return async::Notification();
}
