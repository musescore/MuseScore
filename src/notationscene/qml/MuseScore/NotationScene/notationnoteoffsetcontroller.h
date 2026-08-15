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
#include <optional>
#include <QQuickItem>

#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "notation/notationtypes.h"

namespace mu::engraving {
struct ScoreChanges;
}

namespace mu::notation {
class NoteOffsetOverlay;

class NotationNoteOffsetController : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };

public:
    NotationNoteOffsetController(QQuickItem* overlaysParent, const muse::modularity::ContextPtr& iocCtx);

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

    // Nominal (zero-offset) canvas X positions, taken directly from the note's own layout -
    // anchors the rectangle exactly on the notehead rather than relying on tick interpolation.
    struct NoteEntry {
        mu::engraving::Note* note = nullptr;
        double nominalLeftX = 0.0;
        double nominalRightX = 0.0;
    };

    // Where a given note's rectangle lives, so a drag on a multi-note selection can update/commit
    // every selected note's overlay entry, not just the one under the mouse.
    struct NoteLocation {
        SysStaffKey key;
        int rectIndex = -1;
    };

    // The overlay item, its notes and its canvas-space band rect were previously three separate
    // maps kept in lockstep by every add/remove/clear - a single map to this struct removes the
    // risk of them silently desyncing for a staff.
    struct StaffOverlayData {
        NoteOffsetOverlay* overlay = nullptr;
        std::vector<NoteEntry> notes;
        muse::RectF bandRect;
    };

    using OverlaysMap = std::map<SysStaffKey, StaffOverlayData>;
    using NoteLocationMap = std::map<mu::engraving::Note*, NoteLocation>;

    void rebuildAllOverlays();
    void createOverlayForStaff(const System* system, staff_idx_t staffIdx, OverlaysMap& newOverlays);
    void updateOverlaysGeometry();
    void updateSelectionHighlight();
    void applyOverlayColors(NoteOffsetOverlay* overlay) const;

    void onCurrentNotationChanged();
    void scheduleRebuild();
    void onEdgeDragged(const SysStaffKey& key, int rectIndex, bool isLeftEdge, qreal newXN, bool completed);
    void previewNoteRect(const NoteLocation& location, int newStartOffset, int newDurationOffset);

    std::vector<mu::engraving::Note*> selectedNotes() const;

    INotationNoteOffsetsPtr noteOffsets() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_overlaysParent = nullptr;
    OverlaysMap m_overlaysByStaff;
    NoteLocationMap m_noteLocations;
    muse::draw::Transform m_viewMatrix;
    bool m_rebuildScheduled = false;
};
}
