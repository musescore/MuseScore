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

#include <map>
#include <QElapsedTimer>
#include <QQuickItem>

#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "notation/notationtypes.h"
#include "playback/iplaybackcontroller.h"
#include "notevelocitygeometry.h"

namespace mu::engraving {
struct ScoreChanges;
}

namespace mu::notation {
class NoteVelocityOverlay;

class NotationNoteVelocityController : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };
    muse::GlobalInject<INotationConfiguration> notationConfiguration;
    muse::ContextInject<playback::IPlaybackController> playbackController = { this };

public:
    NotationNoteVelocityController(QQuickItem* overlaysParent, const muse::modularity::ContextPtr& iocCtx);

    void init();
    void setViewMatrix(const muse::draw::Transform& viewMatrix);

private:
    // Necessary since SysStaff doesn't hold a reference to its system, which is needed
    // for calculating a SysStaff's relative position...
    struct SysStaffKey {
        const System* system = nullptr;
        staff_idx_t staffIdx = muse::nidx;

        bool isValid() const
        {
            return system && !system->measures().empty() && staffIdx != muse::nidx;
        }

        bool operator<(const SysStaffKey& k) const
        {
            // Compare the System pointer by address only - never dereference it here. This key
            // is looked up against entries left over from a previous rebuild (to reuse an
            // existing overlay item instead of recreating it), and a view mode switch
            // (Page <-> Continuous) destroys and recreates every System, so a stale key still
            // sitting in the map at that point has a dangling `system` - dereferencing it (as
            // `system->first()->index()` used to) is a use-after-free/crash.
            if (system != k.system) {
                return system < k.system;
            }
            return staffIdx < k.staffIdx;
        }
    };

    // One entry per note. Entries belonging to the same chord are kept contiguous and sorted
    // highest-pitch-first, matching NoteVelocityOverlay's expected back-to-front paint order.
    struct NoteEntry {
        mu::engraving::Note* note = nullptr;
        double leftX = 0.0;
        double rightX = 0.0;
        NoteVelocityYRange yRange;
    };

    struct NoteLocation {
        SysStaffKey key;
        int rectIndex = -1;
    };

    // The overlay item, its notes and its canvas-space band rect were previously three separate
    // maps kept in lockstep by every add/remove/clear - a single map to this struct removes the
    // risk of them silently desyncing for a staff.
    struct StaffOverlayData {
        NoteVelocityOverlay* overlay = nullptr;
        std::vector<NoteEntry> notes;
        muse::RectF bandRect;
    };

    using OverlaysMap = std::map<SysStaffKey, StaffOverlayData>;
    using NoteLocationMap = std::map<mu::engraving::Note*, NoteLocation>;

    void rebuildAllOverlays();
    void createOverlayForStaff(const System* system, staff_idx_t staffIdx, OverlaysMap& newOverlays);
    void updateOverlaysGeometry();
    void updateSelectionHighlight();
    void applyOverlayColors(NoteVelocityOverlay* overlay) const;

    void onCurrentNotationChanged();
    void scheduleRebuild();
    void onBarDragged(const SysStaffKey& key, int rectIndex, qreal deltaYN, bool completed);
    void onDragCancelled(const SysStaffKey& key, int rectIndex);
    void previewBarHeight(const NoteLocation& location, int newVelocity);
    void auditionNote(const mu::engraving::Note* note, int velocity);
    bool auditionThrottleElapsed() const;
    void markAudition(int velocity);
    void resetAuditionThrottle();

    std::vector<mu::engraving::Note*> selectedNotes() const;

    // What the dynamics-marking/hairpin context alone would produce at this note's tick, with no
    // per-note override - used both as the displayed baseline for unedited notes and as the base
    // that a VeloType::OFFSET_VAL note's percentage override applies on top of.
    int contextVelocity(const mu::engraving::Note* note) const;

    // The velocity a note effectively plays at right now: its own explicit override if it has
    // one, otherwise the dynamics-marking/hairpin level alone would produce at its tick - used as
    // the displayed baseline for unedited notes, so nudging one starts from a musically coherent
    // value instead of an arbitrary flat default.
    int displayedVelocity(const mu::engraving::Note* note) const;

    INotationNoteVelocityPtr noteVelocity() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_overlaysParent = nullptr;
    OverlaysMap m_overlaysByStaff;
    NoteLocationMap m_noteLocations;
    muse::draw::Transform m_viewMatrix;
    bool m_rebuildScheduled = false;

    // Avoids re-triggering the audition sound on every single mouse-move event during a drag -
    // only once per actually-distinct velocity value, and never faster than a fixed minimum
    // interval (see AUDITION_MIN_INTERVAL_MS).
    int m_lastAuditionedVelocity = -1;
    QElapsedTimer m_auditionThrottle;
};
}
