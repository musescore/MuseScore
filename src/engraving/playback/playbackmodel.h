/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_PLAYBACKMODEL_H
#define MU_ENGRAVING_PLAYBACKMODEL_H

#include "modularity/ioc.h"
#include "mpe/events.h"
#include "mpe/iarticulationprofilesrepository.h"

#include "playbackeventsrenderer.h"

namespace Ms {
class Score;
}

namespace mu::engraving {
class PlaybackModel
{
    INJECT(engraving, mpe::IArticulationProfilesRepository, profilesRepository)

public:
    PlaybackModel(const Ms::Score* score);

private:
    PlaybackEventsRenderer m_renderer;

    mpe::PlaybackEventList m_events;
};
}

#endif // PLAYBACKMODEL_H
