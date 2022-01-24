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

#include <QString>

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"

using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu::async;

static const std::set<std::string> KEYBOARDS_FAMILY_SET = {
    "keyboards", "organs", "synths",
};

static const std::set<std::string> STRINGS_FAMILY_SET = {
    "harps", "guitars", "bass-guitars", "banjos",
    "ukuleles", "mandolins", "mtn-dulcimers",  "lutes",
    "balalaikas", "bouzoukis", "kotos", "ouds",
    "shamisens", "sitars", "tamburicas", "bandurrias",
    "lauds", "strings", "orchestral-strings", "viols",
    "octobasses", "erhus", "nyckelharpas"
};

static const std::set<std::string> WINDS_FAMILY_SET = {
    "winds", "flutes", "dizis", "shakuhachis",
    "fifes", "whistles", "flageolets", "recorders",
    "ocarinas", "gemshorns", "pan-flutes", "quenas",
    "oboes", "shawms", "cromornes", "crumhorns",
    "cornamuses", "kelhorns", "rauschpfeifes", "duduks",
    "shenais", "clarinets", "chalumeaus", "xaphoons",
    "tarogatos", "octavins", "saxophones", "bassoons",
    "reed-contrabasses", "dulcians", "racketts", "sarrusophones",
    "bagpipes", "accordions", "harmonicas", "melodicas",
    "shengs", "brass", "horns", "wagner-tubas",
    "cornets", "saxhorns", "alto-horns", "baritone-horns",
    "posthorns", "trumpets", "baroque-trumpets", "bugles",
    "flugelhorns", "ophicleides", "cornetts", "serpents",
    "trombones", "sackbuts", "euphoniums", "tubas",
    "sousaphones", "conches", "alphorns", "rag-dungs",
    "didgeridoos", "shofars", "vuvuzelas", "klaxon-horns",
    "kazoos"
};

static const std::set<std::string> PERCUSSION_FAMILY_SET = {
    "timpani", "roto-toms", "tubaphones", "steel-drums",
    "keyboard-percussion", "pitched-metal-percussion",
    "orff-percussion", "flexatones", "musical-saws",
    "glass-percussion", "kalimbas", "drums", "unpitched-metal-percussion",
    "unpitched-wooden-percussion", "other-percussion",
    "batterie", "body-percussion"
};

void PlaybackModel::load(Ms::Score* score, async::Channel<int, int, int, int> notationChangesRangeChannel)
{
    m_score = score;
    notationChangesRangeChannel.resetOnReceive(this);

    notationChangesRangeChannel.onReceive(this, [this](const int tickFrom, const int tickTo,
                                                       const int staffIdxFrom, const int staffIdxTo) {
        clearExpiredEvents();
        clearExpiredContexts();
        update(tickFrom, tickTo, Ms::staff2track(staffIdxFrom, 0), Ms::staff2track(staffIdxTo, Ms::VOICES));
    });

    update(0, score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());
}

const PlaybackEventsMap& PlaybackModel::events(const ID& partId, const std::string& instrumentId) const
{
    return m_events.at(idKey(partId, instrumentId));
}

void PlaybackModel::update(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo)
{
    for (const Ms::RepeatSegment* repeatSegment : m_score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Ms::Measure* measure : repeatSegment->measureList()) {
            int measureStartTick = measure->tick().ticks();
            int measureEndTick = measure->endTick().ticks();

            if (measureStartTick > tickTo || measureEndTick < tickFrom) {
                continue;
            }

            for (Ms::Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentPositionTick = segment->tick().ticks();

                for (int i = trackFrom; i < trackTo; ++i) {
                    Ms::EngravingItem* item = segment->element(i);

                    if (!item || !item->isChordRest() || !item->part()) {
                        continue;
                    }

                    ArticulationsProfilePtr profile = profileByFamily(item->part()->familyId().toStdString());

                    if (!profile) {
                        LOGE() << "unsupported instrument family: " << item->part()->familyId();
                        continue;
                    }

                    TrackIdKey trackId = idKey(item);

                    PlaybackContext& ctx = m_playbackCtxMap[trackId];
                    ctx.update(segment, segmentPositionTick);

                    m_renderer.render(item, tickPositionOffset, ctx.nominalDynamicLevel(segmentPositionTick),
                                      ctx.persistentArticulationType(segmentPositionTick), std::move(profile), m_events[trackId]);
                }
            }
        }
    }
}

void PlaybackModel::clearExpiredEvents()
{
    auto it = m_events.cbegin();

    while (it != m_events.cend())
    {
        const Ms::Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || part->instruments()->contains(it->first.instrumentId)) {
            it = m_events.erase(it);
            continue;
        }

        ++it;
    }
}

void PlaybackModel::clearExpiredContexts()
{
    auto it = m_playbackCtxMap.cbegin();

    while (it != m_playbackCtxMap.cend())
    {
        const Ms::Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || part->instruments()->contains(it->first.instrumentId)) {
            it = m_playbackCtxMap.erase(it);
            continue;
        }

        ++it;
    }
}

PlaybackModel::TrackIdKey PlaybackModel::idKey(const Ms::EngravingItem* item) const
{
    return { item->part()->id(),
             item->part()->instrumentId(item->tick()).toStdString() };
}

PlaybackModel::TrackIdKey PlaybackModel::idKey(const ID& partId, const std::string& instrimentId) const
{
    return { partId, instrimentId };
}

ArticulationsProfilePtr PlaybackModel::profileByFamily(const std::string& familyId) const
{
    if (KEYBOARDS_FAMILY_SET.find(familyId) != KEYBOARDS_FAMILY_SET.cend()) {
        return profilesRepository()->defaultProfile(ArticulationFamily::KeyboardsArticulation);
    }

    if (STRINGS_FAMILY_SET.find(familyId) != STRINGS_FAMILY_SET.cend()) {
        return profilesRepository()->defaultProfile(ArticulationFamily::StringsArticulation);
    }

    if (WINDS_FAMILY_SET.find(familyId) != WINDS_FAMILY_SET.cend()) {
        return profilesRepository()->defaultProfile(ArticulationFamily::WindsArticulation);
    }

    if (PERCUSSION_FAMILY_SET.find(familyId) != PERCUSSION_FAMILY_SET.cend()) {
        return profilesRepository()->defaultProfile(ArticulationFamily::PercussionsArticulation);
    }

    return nullptr;
}
