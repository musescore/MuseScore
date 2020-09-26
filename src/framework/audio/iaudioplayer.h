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

#ifndef MU_AUDIO_IAUDIOPLAYER_H
#define MU_AUDIO_IAUDIOPLAYER_H

#include <memory>
#include "iplayer.h"
#include "iaudiosource.h"
#include "iaudiostream.h"
#include "ret.h"

namespace mu::audio {
class IAudioPlayer : public virtual IPlayer, public virtual IAudioSource
{
public:
    ~IAudioPlayer() = default;

    virtual void unload() = 0;
    virtual Ret load(const std::shared_ptr<audio::IAudioStream>& stream) = 0;
};
}
#endif // MU_AUDIO_IAUDIOPLAYER_H
