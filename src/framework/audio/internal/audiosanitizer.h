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
#ifndef MU_AUDIO_AUDIOSANITIZER_H
#define MU_AUDIO_AUDIOSANITIZER_H

//! NOTE This is dev tools

#include <cassert>

namespace mu::audio {
class AudioSanitizer
{
public:

    static void setupWorkerThread();
    static bool isWorkerThread();
};
}

#define ONLY_AUDIO_WORKER_THREAD assert(mu::audio::AudioSanitizer::isWorkerThread())

#endif // MU_AUDIO_AUDIOSANITIZER_H
