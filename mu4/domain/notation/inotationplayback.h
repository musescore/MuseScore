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
#ifndef MU_DOMAIN_INOTATIONPLAYBACK_H
#define MU_DOMAIN_INOTATIONPLAYBACK_H

#include <QRect>
#include "audio/midi/miditypes.h"

namespace mu {
namespace domain {
namespace notation {
class INotationPlayback
{
public:
    virtual ~INotationPlayback() = default;

    virtual std::shared_ptr<audio::midi::MidiStream> midiStream() const = 0;

    virtual QRect playbackCursorRect(float sec) const = 0;
};
}
}
}

#endif // MU_DOMAIN_INOTATIONPLAYBACK_H
