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

#include "notationnoteoffsetcontroller.h"

#include "noteoffsetoverlay.h"
#include "segmentcanvasinterpolation.h"

#include <algorithm>
#include <optional>

#include "async/async.h"
#include "global/containers.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/property.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationnoteoffsets.h"
#include "notation/inotationselection.h"
#include "notation/inotationstyle.h"
#include "notation/inotationundostack.h"
#include "notation/inotationelements.h" // IWYU pragma: keep

using namespace mu::notation;
using namespace mu::engraving;

// Each rectangle is anchored on its own note's vertical position, not on a fixed lane above the
// staff - this way a rectangle always sits right above its notehead, and chord notes naturally
// stack in the same order as their pitches instead of needing an artificial row index.
constexpr static double RECT_TOP_MARGIN_SP = 0.45; // gap between the notehead center and the rectangle's top edge
constexpr static double RECT_BOTTOM_OVERLAP_SP = 0.4; // how far below the notehead center the rectangle's bottom edge extends

constexpr static int MAX_OFFSET_TICKS = 1920; // matches the Properties panel spinbox range
constexpr static int MIN_EFFECTIVE_TICKS = 1;

static std::optional<int> noteOffsetTickFromCanvasX(const System* system, const muse::RectF& bandCanvasRect, qreal xN)
{
    const double pointCanvasX = bandCanvasRect.x() + xN * bandCanvasRect.width();
    return mu::notation::tickFromCanvasX(system, pointCanvasX);
}

// Pixel shift corresponding to a tick offset away from baseTick, using the same segment
// interpolation as canvasXFromTick/noteOffsetTickFromCanvasX so it round-trips exactly with how
// the mouse position was interpreted. Falls back to a locally-derived ratio only if the note
// sits right at a system boundary where interpolation has nothing to anchor to.
static double pixelDeltaForTickOffset(const System* system, int baseTick, int tickOffset, double fallbackPxPerTick)
{
    if (tickOffset == 0) {
        return 0.0;
    }

    const std::optional<double> basePx = mu::notation::canvasXFromTick(system, baseTick);
    const std::optional<double> offsetPx = mu::notation::canvasXFromTick(system, baseTick + tickOffset);
    if (basePx && offsetPx) {
        return *offsetPx - *basePx;
    }

    return tickOffset * fallbackPxPerTick;
}

NotationNoteOffsetController::NotationNoteOffsetController(QQuickItem* overlaysParent, const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_overlaysParent(overlaysParent)
{
}

void NotationNoteOffsetController::init()
{
    IF_ASSERT_FAILED(noteOffsets() && currentNotation()) {
        return;
    }

    onCurrentNotationChanged();

    noteOffsets()->editModeEnabledChanged().onNotify(this, [this]() {
        if (noteOffsets()->isEditModeEnabled()) {
            rebuildAllOverlays();
        } else {
            updateOverlaysGeometry();
        }
    }, Asyncable::Mode::SetReplace);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    }, Asyncable::Mode::SetReplace);
}

void NotationNoteOffsetController::onCurrentNotationChanged()
{
    rebuildAllOverlays();

    if (mu::engraving::Score* thisScore = score()) {
        // TODO: More efficient if we only rebuild the affected staves/systems...
        // SetReplace only dedupes a subscription against the exact same Score/Notation instance -
        // switching documents subscribes to a brand new instance each time, so guard the callback
        // itself against firing for a document that's no longer current, rather than leaking one
        // live subscription per every document ever opened this session.
        score()->changesChannel().onReceive(this, [this, thisScore](const mu::engraving::ScoreChanges&) {
            if (thisScore != score()) {
                return;
            }
            scheduleRebuild();
        }, Asyncable::Mode::SetReplace);
    }

    const INotationPtr notation = currentNotation();
    if (notation) {
        mu::notation::INotation* thisNotation = notation.get();

        // Switching between Page/Continuous/Continuous vertical view completely re-flows the
        // systems - the overlays' cached positions need to be rebuilt from scratch, not just
        // repositioned via the view matrix.
        notation->viewModeChanged().onNotify(this, [this, thisNotation]() {
            if (thisNotation != currentNotation().get()) {
                return;
            }
            scheduleRebuild();
        }, Asyncable::Mode::SetReplace);

        if (notation->style()) {
            // Style edits (e.g. live-dragging "Staff space (sp)" in Page Settings) relayout the
            // score without necessarily going through changesChannel() - without this, the
            // overlay's cached note positions go stale and stop tracking the rescaled notation.
            notation->style()->styleChanged().onNotify(this, [this, thisNotation]() {
                if (thisNotation != currentNotation().get()) {
                    return;
                }
                scheduleRebuild();
            }, Asyncable::Mode::SetReplace);
        }

        if (notation->interaction()) {
            notation->interaction()->selectionChanged().onNotify(this, [this, thisNotation]() {
                if (thisNotation != currentNotation().get()) {
                    return;
                }
                updateSelectionHighlight();
            }, Asyncable::Mode::SetReplace);
        }
    }
}

void NotationNoteOffsetController::scheduleRebuild()
{
    if (m_rebuildScheduled) {
        return;
    }
    m_rebuildScheduled = true;

    // Defer to the next event loop iteration - the score may still be mid-layout at the
    // point the changesChannel notification fires, so rebuilding synchronously here (which
    // reads System/Segment/Chord layout data) is not safe.
    muse::async::Async::call(this, [this]() {
        m_rebuildScheduled = false;
        if (noteOffsets() && noteOffsets()->isEditModeEnabled()) {
            rebuildAllOverlays();
        }
    });
}

void NotationNoteOffsetController::rebuildAllOverlays()
{
    for (const auto& [key, data] : m_overlaysByStaff) {
        if (data.overlay->isDragging()) {
            // Deleting an overlay that currently holds the mouse grab (mid-drag) would drop the
            // in-progress edit and risk delivering the next mouse event to a freed item - wait
            // for the drag to finish instead of rebuilding out from under it.
            scheduleRebuild();
            return;
        }
    }

    m_noteLocations.clear();

    if (!score()) {
        // Happens on close...
        for (const auto& [key, data] : m_overlaysByStaff) {
            delete data.overlay;
        }
        m_overlaysByStaff.clear();
        return;
    }

    // createOverlayForStaff reuses an existing overlay item in place (just updating its rects)
    // when a staff already had one, instead of destroying and recreating every overlay QQuickItem
    // on every edit - it consumes matching entries out of m_overlaysByStaff as it goes, so
    // whatever is left there afterwards belongs to a staff that's no longer visible/primary/has
    // no offsettable notes anymore, and can be deleted.
    OverlaysMap newOverlays;

    for (const System* system : score()->systems()) {
        staff_idx_t staffIdx = system->firstVisibleStaff();
        while (staffIdx != muse::nidx) {
            createOverlayForStaff(system, staffIdx, newOverlays);
            staffIdx = system->nextVisibleStaff(staffIdx);
        }
    }

    for (const auto& [key, data] : m_overlaysByStaff) {
        delete data.overlay;
    }

    m_overlaysByStaff = std::move(newOverlays);

    updateOverlaysGeometry();
}

void NotationNoteOffsetController::createOverlayForStaff(const System* system, staff_idx_t staffIdx, OverlaysMap& newOverlays)
{
    IF_ASSERT_FAILED(system && m_overlaysParent && score()) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    const SysStaff* sysStaff = system->staff(staffIdx);
    if (!staff || !sysStaff || !staff->isPrimaryStaff()) {
        return;
    }

    std::vector<NoteEntry> entries;

    const track_idx_t strack = staffIdx * VOICES;
    const track_idx_t etrack = strack + VOICES;

    for (const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(SegmentType::ChordRest) : nullptr;
         seg && seg->system() == system; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* item = seg->element(track);
            if (!item || !item->isChord()) {
                continue;
            }
            const Chord* chord = toChord(item);

            // The nominal (zero-offset) right edge is anchored on this chord's own end tick, not
            // on "the next ChordRest segment" - that segment is shared across every voice/track at
            // this staff, so a shorter simultaneous note in another voice would otherwise cut this
            // chord's rectangle down to the shorter note's end tick instead of its own.
            const int chordEndTick = chord->tick().ticks() + chord->ticks().ticks();
            const std::optional<double> interpolatedRightX = mu::notation::canvasXFromTick(system, chordEndTick);
            const double nominalRightX = interpolatedRightX ? *interpolatedRightX : (seg->canvasX() + seg->width());

            for (Note* note : chord->notes()) {
                if (note->tieBack()) {
                    // Playback (NoteRenderer::shouldRender) skips tied-continuation notes
                    // entirely in most cases, so their own offset would silently do nothing -
                    // don't offer a handle that can't actually affect anything.
                    continue;
                }

                NoteEntry entry;
                entry.note = note;
                entry.nominalLeftX = note->canvasX();
                entry.nominalRightX = nominalRightX;
                entries.push_back(entry);
            }
        }
    }

    if (entries.empty()) {
        return;
    }

    const double spatium = entries.front().note->spatium();
    const double topMargin = RECT_TOP_MARGIN_SP * spatium;
    const double bottomOverlap = RECT_BOTTOM_OVERLAP_SP * spatium;
    const double rectHeight = topMargin + bottomOverlap;
    const double vPadding = 0.3 * spatium;

    // Anchored on each note's own vertical position, so the rectangle sits right above its
    // notehead (and chord notes stack in pitch order without needing an artificial row index)
    std::vector<double> centerY;
    centerY.reserve(entries.size());
    double minY = 0.0;
    double maxY = 0.0;
    for (size_t i = 0; i < entries.size(); ++i) {
        const double noteY = entries[i].note->canvasPos().y();
        const double y = noteY - topMargin + rectHeight / 2.0;
        centerY.push_back(y);
        if (i == 0) {
            minY = noteY - topMargin;
            maxY = noteY + bottomOverlap;
        } else {
            minY = std::min(minY, noteY - topMargin);
            maxY = std::max(maxY, noteY + bottomOverlap);
        }
    }
    minY -= vPadding;
    maxY += vPadding;

    // The overlay's vertical bounds are derived from the actual note positions rather than a
    // fixed margin around the staff - this way it always contains every rectangle regardless of
    // how far above/below the staff a note sits (ledger lines, etc.)
    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const muse::RectF overlayCanvasRect(staffCanvasRect.x(), minY, staffCanvasRect.width(), maxY - minY);

    const std::vector<Note*> selected = selectedNotes();

    QVector<NoteOffsetOverlay::RectData> rects;
    rects.reserve(static_cast<int>(entries.size()));

    for (size_t i = 0; i < entries.size(); ++i) {
        const NoteEntry& entry = entries[i];
        const Note* note = entry.note;
        const Chord* chord = note->chord();
        IF_ASSERT_FAILED(chord) {
            continue;
        }

        // Fallback local px-per-tick rate, only used if a note's offset pushes it right at a
        // system boundary where segment interpolation has nothing to anchor to.
        const int chordTicks = chord->ticks().ticks();
        const double fallbackPxPerTick = chordTicks > 0 ? (entry.nominalRightX - entry.nominalLeftX) / chordTicks : 0.0;

        const int chordStartTick = chord->tick().ticks();
        const int chordEndTick = chordStartTick + chordTicks;
        const double leftPx = entry.nominalLeftX
                              + pixelDeltaForTickOffset(system, chordStartTick, note->playbackStartOffset(), fallbackPxPerTick);
        const double rightPx = entry.nominalRightX
                               + pixelDeltaForTickOffset(system, chordEndTick, note->playbackDurationOffset(), fallbackPxPerTick);

        NoteOffsetOverlay::RectData rect;
        rect.leftN = (leftPx - overlayCanvasRect.x()) / overlayCanvasRect.width();
        rect.rightN = (rightPx - overlayCanvasRect.x()) / overlayCanvasRect.width();
        rect.centerYN = (centerY[i] - overlayCanvasRect.y()) / overlayCanvasRect.height();
        rect.heightYN = rectHeight / overlayCanvasRect.height();
        rect.selected = muse::contains(selected, entry.note);
        rect.userModified = note->playbackStartOffset() != 0 || note->playbackDurationOffset() != 0;
        rects.push_back(rect);
    }

    if (rects.isEmpty()) {
        return;
    }

    const SysStaffKey key { system, staffIdx };
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        m_noteLocations[entries[i].note] = NoteLocation { key, i };
    }

    NoteOffsetOverlay* overlay = nullptr;
    const auto oldIt = m_overlaysByStaff.find(key);
    if (oldIt != m_overlaysByStaff.end()) {
        // Reuse the existing overlay item in place rather than destroying and recreating it -
        // its drag-signal connection (bound to this same key) is still valid.
        overlay = oldIt->second.overlay;
        overlay->setRects(rects);
        m_overlaysByStaff.erase(oldIt);
    } else {
        overlay = new NoteOffsetOverlay(m_overlaysParent);
        overlay->setRects(rects);
        applyOverlayColors(overlay);
        overlay->setVisible(false);

        QObject::connect(overlay, &NoteOffsetOverlay::edgeDragged,
                         [this, key](int rectIndex, bool isLeftEdge, qreal newXN, bool completed) {
            onEdgeDragged(key, rectIndex, isLeftEdge, newXN, completed);
        });
    }

    StaffOverlayData data;
    data.overlay = overlay;
    data.notes = std::move(entries);
    data.bandRect = overlayCanvasRect;
    newOverlays[key] = std::move(data);
}

void NotationNoteOffsetController::applyOverlayColors(NoteOffsetOverlay* overlay) const
{
    IF_ASSERT_FAILED(overlay) {
        return;
    }

    overlay->setFillColor(QColor(90, 180, 140, 60));
    overlay->setSelectedFillColor(QColor(60, 160, 210, 90));
    overlay->setModifiedFillColor(QColor(235, 140, 40, 90));
    overlay->setBorderColor(QColor(50, 130, 100, 200));
    overlay->setHandleColor(QColor(90, 180, 140, 230).darker(160));
    overlay->setSelectedHandleColor(QColor(60, 160, 210, 230).darker(140));
    overlay->setModifiedHandleColor(QColor(235, 140, 40, 230).darker(140));
}

void NotationNoteOffsetController::updateSelectionHighlight()
{
    if (!noteOffsets() || !noteOffsets()->isEditModeEnabled()) {
        return;
    }

    const std::vector<Note*> selected = selectedNotes();

    for (const auto& [key, data] : m_overlaysByStaff) {
        const QVector<NoteOffsetOverlay::RectData>& rects = data.overlay->rects();
        if (rects.size() != static_cast<int>(data.notes.size())) {
            continue;
        }

        // Only a handful of notes typically change selection at once, even on a staff with many
        // notes - update just those rects in place instead of copying the whole vector out and
        // back regardless of how many actually changed.
        for (int i = 0; i < rects.size(); ++i) {
            const bool isSelected = muse::contains(selected, data.notes.at(i).note);
            if (rects.at(i).selected != isSelected) {
                NoteOffsetOverlay::RectData rect = rects.at(i);
                rect.selected = isSelected;
                data.overlay->updateRect(i, rect);
            }
        }
    }
}

void NotationNoteOffsetController::updateOverlaysGeometry()
{
    const bool visible = noteOffsets() && noteOffsets()->isEditModeEnabled();

    for (const auto& [key, data] : m_overlaysByStaff) {
        data.overlay->setVisible(visible);
        if (!visible) {
            continue;
        }

        const muse::RectF screenRect = m_viewMatrix.map(data.bandRect);
        data.overlay->setWidth(screenRect.width());
        data.overlay->setHeight(screenRect.height());
        data.overlay->setX(screenRect.x());
        data.overlay->setY(screenRect.y());
    }
}

void NotationNoteOffsetController::setViewMatrix(const muse::draw::Transform& viewMatrix)
{
    if (viewMatrix == m_viewMatrix) {
        return;
    }
    m_viewMatrix = viewMatrix;

    if (noteOffsets() && noteOffsets()->isEditModeEnabled()) {
        updateOverlaysGeometry();
    }
}

std::vector<mu::engraving::Note*> NotationNoteOffsetController::selectedNotes() const
{
    const INotationPtr notation = currentNotation();
    if (!notation || !notation->interaction() || !notation->interaction()->selection()) {
        return {};
    }

    return notation->interaction()->selection()->notes();
}

void NotationNoteOffsetController::previewNoteRect(const NoteLocation& location, int newStartOffset, int newDurationOffset)
{
    const auto dataIt = m_overlaysByStaff.find(location.key);
    IF_ASSERT_FAILED(dataIt != m_overlaysByStaff.end() && location.rectIndex >= 0
                     && static_cast<size_t>(location.rectIndex) < dataIt->second.notes.size()) {
        return;
    }
    const StaffOverlayData& data = dataIt->second;

    const NoteEntry& entry = data.notes.at(location.rectIndex);
    const Chord* chord = entry.note ? entry.note->chord() : nullptr;
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const int chordTicks = chord->ticks().ticks();
    const double fallbackPxPerTick = chordTicks > 0 ? (entry.nominalRightX - entry.nominalLeftX) / chordTicks : 0.0;
    const int chordStartTick = chord->tick().ticks();
    const int chordEndTick = chordStartTick + chordTicks;
    const double leftPx = entry.nominalLeftX
                          + pixelDeltaForTickOffset(location.key.system, chordStartTick, newStartOffset, fallbackPxPerTick);
    const double rightPx = entry.nominalRightX
                           + pixelDeltaForTickOffset(location.key.system, chordEndTick, newDurationOffset, fallbackPxPerTick);

    const QVector<NoteOffsetOverlay::RectData>& rects = data.overlay->rects();
    if (location.rectIndex >= rects.size()) {
        return;
    }

    // Single-struct copy plus an in-place update, instead of copying the whole staff's rect
    // vector out and back on every mouse-move during a drag.
    NoteOffsetOverlay::RectData rect = rects.at(location.rectIndex);
    rect.leftN = (leftPx - data.bandRect.x()) / data.bandRect.width();
    rect.rightN = (rightPx - data.bandRect.x()) / data.bandRect.width();
    data.overlay->updateRect(location.rectIndex, rect);
}

void NotationNoteOffsetController::onEdgeDragged(const SysStaffKey& key, int rectIndex, bool isLeftEdge, qreal newXN, bool completed)
{
    const auto dataIt = m_overlaysByStaff.find(key);
    IF_ASSERT_FAILED(key.isValid() && dataIt != m_overlaysByStaff.end()
                     && rectIndex >= 0 && static_cast<size_t>(rectIndex) < dataIt->second.notes.size()) {
        return;
    }
    const StaffOverlayData& data = dataIt->second;

    const NoteEntry& draggedEntry = data.notes.at(rectIndex);
    Note* draggedNote = draggedEntry.note;
    Chord* draggedChord = draggedNote ? draggedNote->chord() : nullptr;
    IF_ASSERT_FAILED(draggedNote && draggedChord) {
        return;
    }

    const std::optional<int> newTick = noteOffsetTickFromCanvasX(key.system, data.bandRect, newXN);
    if (!newTick) {
        return;
    }

    const int draggedChordStartTick = draggedChord->tick().ticks();
    const int draggedChordEndTick = draggedChordStartTick + draggedChord->ticks().ticks();

    int newStartOffset = draggedNote->playbackStartOffset();
    int newDurationOffset = draggedNote->playbackDurationOffset();

    if (isLeftEdge) {
        newStartOffset = std::clamp(*newTick - draggedChordStartTick, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
        const int effEnd = draggedChordEndTick + newDurationOffset;
        if (effEnd - (draggedChordStartTick + newStartOffset) < MIN_EFFECTIVE_TICKS) {
            newStartOffset = std::clamp(effEnd - MIN_EFFECTIVE_TICKS - draggedChordStartTick, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
        }
    } else {
        newDurationOffset = std::clamp(*newTick - draggedChordEndTick, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
        const int effStart = draggedChordStartTick + newStartOffset;
        if ((draggedChordEndTick + newDurationOffset) - effStart < MIN_EFFECTIVE_TICKS) {
            newDurationOffset = std::clamp(effStart + MIN_EFFECTIVE_TICKS - draggedChordEndTick, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
        }
    }

    // If the dragged note is part of a multi-note selection, apply the same tick delta to every
    // other selected note's corresponding offset, each clamped independently.
    const int delta = isLeftEdge ? (newStartOffset - draggedNote->playbackStartOffset())
                      : (newDurationOffset - draggedNote->playbackDurationOffset());

    std::vector<Note*> affectedNotes { draggedNote };
    if (delta != 0 || !completed) {
        const std::vector<Note*> selected = selectedNotes();
        if (selected.size() > 1 && muse::contains(selected, draggedNote)) {
            affectedNotes = selected;
        }
    }

    struct PendingChange {
        Note* note = nullptr;
        int startOffset = 0;
        int durationOffset = 0;
    };
    std::vector<PendingChange> changes;
    changes.reserve(affectedNotes.size());

    for (Note* note : affectedNotes) {
        if (note == draggedNote) {
            changes.push_back({ note, newStartOffset, newDurationOffset });
            continue;
        }

        const Chord* chord = note->chord();
        if (!chord) {
            continue;
        }

        int otherStartOffset = note->playbackStartOffset();
        int otherDurationOffset = note->playbackDurationOffset();

        if (isLeftEdge) {
            otherStartOffset = std::clamp(otherStartOffset + delta, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
            const int chordEndTick = chord->tick().ticks() + chord->ticks().ticks();
            const int effEnd = chordEndTick + otherDurationOffset;
            if (effEnd - (chord->tick().ticks() + otherStartOffset) < MIN_EFFECTIVE_TICKS) {
                otherStartOffset = std::clamp(effEnd - MIN_EFFECTIVE_TICKS - chord->tick().ticks(),
                                              -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
            }
        } else {
            otherDurationOffset = std::clamp(otherDurationOffset + delta, -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
            const int chordStartTick = chord->tick().ticks();
            const int chordEndTick = chordStartTick + chord->ticks().ticks();
            const int effStart = chordStartTick + otherStartOffset;
            if ((chordEndTick + otherDurationOffset) - effStart < MIN_EFFECTIVE_TICKS) {
                otherDurationOffset = std::clamp(effStart + MIN_EFFECTIVE_TICKS - chordEndTick,
                                                 -MAX_OFFSET_TICKS, MAX_OFFSET_TICKS);
            }
        }

        changes.push_back({ note, otherStartOffset, otherDurationOffset });
    }

    if (!completed) {
        // Live drag preview - update every affected overlay's displayed rect without touching
        // the score, anchored on the same nominal note positions used when overlays were built
        for (const PendingChange& change : changes) {
            const auto locIt = m_noteLocations.find(change.note);
            if (locIt != m_noteLocations.end()) {
                previewNoteRect(locIt->second, change.startOffset, change.durationOffset);
            }
        }
        return;
    }

    const INotationPtr notation = currentNotation();
    const INotationUndoStackPtr undoStack = notation ? notation->undoStack() : nullptr;
    IF_ASSERT_FAILED(undoStack) {
        return;
    }

    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Change note playback offset"));
    for (const PendingChange& change : changes) {
        if (isLeftEdge) {
            change.note->undoChangeProperty(mu::engraving::Pid::PLAYBACK_START_OFFSET, change.startOffset,
                                            mu::engraving::PropertyFlags::NOSTYLE);
        } else {
            change.note->undoChangeProperty(mu::engraving::Pid::PLAYBACK_DURATION_OFFSET, change.durationOffset,
                                            mu::engraving::PropertyFlags::NOSTYLE);
        }
    }
    undoStack->commitChanges();
}

INotationNoteOffsetsPtr NotationNoteOffsetController::noteOffsets() const
{
    const IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();
    return masterNotation ? masterNotation->noteOffsets() : nullptr;
}

INotationPtr NotationNoteOffsetController::currentNotation() const
{
    return globalContext()->currentNotation();
}

mu::engraving::Score* NotationNoteOffsetController::score() const
{
    return currentNotation() ? currentNotation()->elements()->msScore() : nullptr;
}
