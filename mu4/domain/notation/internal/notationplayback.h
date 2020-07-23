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
#ifndef MU_DOMAIN_NOTATIONPLAYBACK_H
#define MU_DOMAIN_NOTATIONPLAYBACK_H

#include "../inotationplayback.h"
#include "igetscore.h"
#include "async/asyncable.h"

namespace Ms {
class EventMap;
}

namespace mu {
namespace domain {
namespace notation {
class NotationPlayback : public INotationPlayback, public async::Asyncable
{
public:
    NotationPlayback(IGetScore* getScore);

    std::shared_ptr<audio::midi::MidiStream> midiStream() const override;

    QRect playbackCursorRect(float sec) const override;

private:

    struct ChanInfo {
        size_t trackIdx = 0;
        uint16_t bank = 0;
        uint16_t program = 0;
    };

    struct MetaInfo {
        size_t tracksCount = 0;
        std::map<uint16_t, ChanInfo> channels;
    };

    void makeInitData(audio::midi::MidiData& data, Ms::Score* score) const;
    void makeEventMap(Ms::EventMap& eventMap, Ms::Score* score) const;
    void makeMetaInfo(MetaInfo& meta, const Ms::Score* score) const;
    void fillTracks(std::vector<audio::midi::Track>& tracks, const Ms::EventMap& eventMap, const MetaInfo& meta) const;
    void fillTempoMap(std::map<uint32_t /*tick*/, uint32_t /*tempo*/>& tempos, const Ms::Score* score) const;

    IGetScore* m_getScore = nullptr;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONPLAYBACK_H
