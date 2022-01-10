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

#include "playbackmodel.h"

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/segment.h"

using namespace mu::engraving;
using namespace mu::mpe;

PlaybackModel::PlaybackModel(const Ms::Score* score)
{
    NOT_IMPLEMENTED;

    m_events.reserve(15000);

    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(ArticulationFamily::StringsArticulation);

    for (const Ms::RepeatSegment* repeatSegment : score->repeatList()) {
        for (const Ms::Measure* measure : repeatSegment->measureList()) {
            for (Ms::Segment* segment = measure->first(); segment; segment = segment->next()) {
                for (int i = 0; i < score->ntracks(); ++i) {
                    Ms::EngravingItem* item = segment->element(i);

                    if (!item || !item->isChordRest()) {
                        continue;
                    }

                    m_renderer.render(item, dynamicLevelFromType(mpe::DynamicType::Natural), profile, m_events);
                }
            }
        }
    }
}
