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
#ifndef MU_AUDIO_AUDIOENGINEDEVTOOLS_H
#define MU_AUDIO_AUDIOENGINEDEVTOOLS_H

#include <QObject>

#include "modularity/ioc.h"
#include "audio/engine/iaudioengine.h"
#include "sinestream.h"

namespace mu {
namespace audio {
namespace engine {
class AudioEngineDevTools : public QObject
{
    Q_OBJECT
    INJECT(audio, IAudioEngine, engine)

public:
    explicit AudioEngineDevTools(QObject* parent = nullptr);

    Q_INVOKABLE void playSine();
    Q_INVOKABLE void stopSine();

private:

    SineStream m_sineStream;
    IAudioEngine::handle m_sineHandel = 0;
};
}
}
}

#endif // MU_AUDIO_AUDIOENGINEDEVTOOLS_H
