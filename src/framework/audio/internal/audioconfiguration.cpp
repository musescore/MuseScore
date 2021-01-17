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
#include "audioconfiguration.h"
#include "settings.h"

//TODO: remove with global clearing of Q_OS_*** defines
#include <QtGlobal>

using namespace mu::framework;
using namespace mu::audio;

//TODO: add other setting: audio device etc
static const Settings::Key AUDIO_BUFFER_SIZE("audio", "driver_buffer");

void AudioConfiguration::init()
{
    int defaultBufferSize = 0;
#ifdef Q_OS_WASM
    defaultBufferSize = 8192;
#else
    defaultBufferSize = 1024;
#endif
    settings()->setDefaultValue(AUDIO_BUFFER_SIZE, Val(defaultBufferSize));
}

unsigned int AudioConfiguration::driverBufferSize() const
{
    return settings()->value(AUDIO_BUFFER_SIZE).toInt();
}
