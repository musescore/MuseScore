/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include "layoutcontext.h"

#include "dom/sharedpart.h"

namespace mu::engraving::rendering::score {
class StaveSharingLayout
{
public:
    static void updateStaveSharingForFullSystem(MeasureBase* firstMB, MeasureBase* lastMB, LayoutContext& ctx);
    static void updateStaveSharingForLastAddedMeasure(System* system, LayoutContext& ctx);

private:
    using TrackGroup = std::vector<track_idx_t>;

    struct StaveSharingContext {
        bool updateForLastAdded = true;
        bool trackMapChanged = false;
        Fraction sTick = Fraction(0, 1);
        Fraction eTick = Fraction(0, 1);
        std::vector<Segment*> allSegments;
        std::vector<Segment*> crSegments;
        std::vector<Segment*> segmentsToUpdate;
        std::vector<Segment*> crSegmentsToUpdate;

        Score* score = nullptr;
        LayoutContext& layoutCtx;

        StaveSharingContext(MeasureBase* first, MeasureBase* last, LayoutContext& ctx);
    };

    static void updateStaveSharing(StaveSharingContext& ctx);

    static void updateTrackMaps(SharedPart* p, StaveSharingContext& ctx);
    static SharedTrackMap computeTrackMap(SharedPart* p, StaveSharingContext& ctx);

    static bool isEmpty(track_idx_t track, StaveSharingContext& ctx);

    static bool isUnison(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx);
    static bool canGoToSameVoice(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx, const TrackGroup& curTrackGroup,
                                 std::unordered_set<Note*>& localUnisonNotes);
    static bool checkAnnotationsForSameVoice(Segment* segment, track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx);
    static bool canGoToSameStave(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx);

    static void updateNotation(SharedPart* p, StaveSharingContext& ctx);
    static void computeSegmentsToUpdate(StaveSharingContext& ctx);
    static void disconnectAll(SharedPart* p, StaveSharingContext& ctx);

    static void makeSharedNotation(SharedPart* p, StaveSharingContext& ctx);
    static void makeSharedChordRests(SharedPart* p, StaveSharingContext& ctx);
    static void makeSharedAnnotations(SharedPart* p, StaveSharingContext& ctx);
    static bool annotationRefersToBothVoices(EngravingItem* sharedAnnotation, StaveSharingContext& ctx);

    static void cleanup(SharedPart* p, StaveSharingContext& ctx);
};
}
