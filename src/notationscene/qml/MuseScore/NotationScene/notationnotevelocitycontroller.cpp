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

#include "notationnotevelocitycontroller.h"

#include "notevelocityoverlay.h"

#include <algorithm>
#include <cmath>

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
#include "engraving/types/types.h"

#include "mpe/mpetypes.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationnotevelocity.h"
#include "notation/inotationplayback.h"
#include "notation/inotationselection.h"
#include "notation/inotationstyle.h"
#include "notation/inotationundostack.h"
#include "notation/inotationelements.h" // IWYU pragma: keep

using namespace mu::notation;
using namespace mu::engraving;

// Reserve velocity 0 for the model's own "no override, fall back to the dynamic marking" sentinel
// (Note::userVelocity() == 0) - the overlay itself always writes an explicit absolute value, so it
// never produces that sentinel by accident.
constexpr static int MIN_DRAGGABLE_VELOCITY = 1;
constexpr static int MAX_DRAGGABLE_VELOCITY = 127;

constexpr static double BAR_HALF_WIDTH_SP = 0.45;
constexpr static double BAND_V_PADDING_SP = 0.3;


NotationNoteVelocityController::NotationNoteVelocityController(QQuickItem* overlaysParent, const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_overlaysParent(overlaysParent)
{
}

void NotationNoteVelocityController::init()
{
    IF_ASSERT_FAILED(noteVelocity() && currentNotation()) {
        return;
    }

    onCurrentNotationChanged();

    noteVelocity()->editModeEnabledChanged().onNotify(this, [this]() {
        if (noteVelocity()->isEditModeEnabled()) {
            rebuildAllOverlays();
        } else {
            updateOverlaysGeometry();
        }
    }, Asyncable::Mode::SetReplace);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    }, Asyncable::Mode::SetReplace);
}

void NotationNoteVelocityController::onCurrentNotationChanged()
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

void NotationNoteVelocityController::scheduleRebuild()
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
        if (noteVelocity() && noteVelocity()->isEditModeEnabled()) {
            rebuildAllOverlays();
        }
    });
}

void NotationNoteVelocityController::rebuildAllOverlays()
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
    // no notes anymore, and can be deleted.
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

void NotationNoteVelocityController::createOverlayForStaff(const System* system, staff_idx_t staffIdx, OverlaysMap& newOverlays)
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

            std::vector<Note*> chordNotes = chord->notes();
            // Highest pitch first - matches NoteVelocityOverlay's expected back-to-front paint
            // order, so chord notes stack with the lowest-pitched note's bar fully in front.
            std::sort(chordNotes.begin(), chordNotes.end(), [](const Note* a, const Note* b) {
                return a->line() < b->line();
            });

            for (Note* note : chordNotes) {
                if (note->tieBack()) {
                    // Playback (NoteRenderer::shouldRender) skips tied-continuation notes
                    // entirely in most cases, so their own velocity would silently do nothing -
                    // don't offer a handle that can't actually affect anything.
                    continue;
                }

                NoteEntry entry;
                entry.note = note;
                entry.leftX = note->canvasX() - BAR_HALF_WIDTH_SP * note->spatium();
                entry.rightX = note->canvasX() + BAR_HALF_WIDTH_SP * note->spatium();
                entry.yRange = noteVelocityYRange(note);
                entries.push_back(entry);
            }
        }
    }

    if (entries.empty()) {
        return;
    }

    const double vPadding = BAND_V_PADDING_SP * entries.front().note->spatium();
    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());

    double minY = staffCanvasRect.top();
    double maxY = staffCanvasRect.bottom();
    for (const NoteEntry& entry : entries) {
        minY = std::min({ minY, entry.yRange.y0, entry.yRange.y127 });
        maxY = std::max({ maxY, entry.yRange.y0, entry.yRange.y127 });
    }
    minY -= vPadding;
    maxY += vPadding;

    const muse::RectF overlayCanvasRect(staffCanvasRect.x(), minY, staffCanvasRect.width(), maxY - minY);

    const std::vector<Note*> selected = selectedNotes();

    QVector<NoteVelocityOverlay::RectData> rects;
    rects.reserve(static_cast<int>(entries.size()));

    for (const NoteEntry& entry : entries) {
        NoteVelocityOverlay::RectData rect;
        rect.leftN = (entry.leftX - overlayCanvasRect.x()) / overlayCanvasRect.width();
        rect.rightN = (entry.rightX - overlayCanvasRect.x()) / overlayCanvasRect.width();
        rect.y0N = (entry.yRange.y0 - overlayCanvasRect.y()) / overlayCanvasRect.height();
        const double initialTopY = canvasYFromVelocity(entry.yRange, displayedVelocity(entry.note));
        rect.yTopN = (initialTopY - overlayCanvasRect.y()) / overlayCanvasRect.height();
        rect.selected = muse::contains(selected, entry.note);
        rect.userModified = entry.note->userVelocity() != 0;
        rects.push_back(rect);
    }

    const SysStaffKey key { system, staffIdx };
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        m_noteLocations[entries[i].note] = NoteLocation { key, i };
    }

    NoteVelocityOverlay* overlay = nullptr;
    const auto oldIt = m_overlaysByStaff.find(key);
    if (oldIt != m_overlaysByStaff.end()) {
        // Reuse the existing overlay item in place rather than destroying and recreating it -
        // its drag-signal connection (bound to this same key) is still valid.
        overlay = oldIt->second.overlay;
        overlay->setRects(rects);
        m_overlaysByStaff.erase(oldIt);
    } else {
        overlay = new NoteVelocityOverlay(m_overlaysParent);
        overlay->setRects(rects);
        applyOverlayColors(overlay);
        overlay->setVisible(false);

        QObject::connect(overlay, &NoteVelocityOverlay::barDragged, [this, key](int rectIndex, qreal newYN, bool completed) {
            onBarDragged(key, rectIndex, newYN, completed);
        });
    }

    StaffOverlayData data;
    data.overlay = overlay;
    data.notes = std::move(entries);
    data.bandRect = overlayCanvasRect;
    newOverlays[key] = std::move(data);
}

void NotationNoteVelocityController::applyOverlayColors(NoteVelocityOverlay* overlay) const
{
    IF_ASSERT_FAILED(overlay) {
        return;
    }

    overlay->setFillColor(QColor(90, 180, 140, 220));
    overlay->setSelectedFillColor(QColor(60, 160, 210, 235));
    overlay->setModifiedFillColor(QColor(235, 140, 40, 230));
    overlay->setBorderColor(QColor(50, 130, 100, 255));
}

void NotationNoteVelocityController::updateOverlaysGeometry()
{
    const bool visible = noteVelocity() && noteVelocity()->isEditModeEnabled();

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

void NotationNoteVelocityController::updateSelectionHighlight()
{
    if (!noteVelocity() || !noteVelocity()->isEditModeEnabled()) {
        return;
    }

    const std::vector<Note*> selected = selectedNotes();

    for (const auto& [key, data] : m_overlaysByStaff) {
        const QVector<NoteVelocityOverlay::RectData>& rects = data.overlay->rects();
        if (rects.size() != static_cast<int>(data.notes.size())) {
            continue;
        }

        // Only a handful of notes typically change selection at once, even on a staff with many
        // notes - update just those rects in place instead of copying the whole vector out and
        // back regardless of how many actually changed.
        for (int i = 0; i < rects.size(); ++i) {
            const bool isSelected = muse::contains(selected, data.notes.at(i).note);
            if (rects.at(i).selected != isSelected) {
                NoteVelocityOverlay::RectData rect = rects.at(i);
                rect.selected = isSelected;
                data.overlay->updateRect(i, rect);
            }
        }
    }
}

void NotationNoteVelocityController::setViewMatrix(const muse::draw::Transform& viewMatrix)
{
    if (viewMatrix == m_viewMatrix) {
        return;
    }
    m_viewMatrix = viewMatrix;

    if (noteVelocity() && noteVelocity()->isEditModeEnabled()) {
        updateOverlaysGeometry();
    }
}

std::vector<mu::engraving::Note*> NotationNoteVelocityController::selectedNotes() const
{
    const INotationPtr notation = currentNotation();
    if (!notation || !notation->interaction() || !notation->interaction()->selection()) {
        return {};
    }

    return notation->interaction()->selection()->notes();
}

void NotationNoteVelocityController::previewBarHeight(const NoteLocation& location, int newVelocity)
{
    const auto dataIt = m_overlaysByStaff.find(location.key);
    IF_ASSERT_FAILED(dataIt != m_overlaysByStaff.end() && location.rectIndex >= 0
                     && static_cast<size_t>(location.rectIndex) < dataIt->second.notes.size()) {
        return;
    }
    const StaffOverlayData& data = dataIt->second;

    const NoteEntry& entry = data.notes.at(location.rectIndex);
    const double newTopY = canvasYFromVelocity(entry.yRange, newVelocity);

    const QVector<NoteVelocityOverlay::RectData>& rects = data.overlay->rects();
    if (location.rectIndex >= rects.size()) {
        return;
    }

    // Single-struct copy plus an in-place update, instead of copying the whole staff's rect
    // vector out and back on every mouse-move during a drag.
    NoteVelocityOverlay::RectData rect = rects.at(location.rectIndex);
    rect.yTopN = (newTopY - data.bandRect.y()) / data.bandRect.height();
    data.overlay->updateRect(location.rectIndex, rect);
}

void NotationNoteVelocityController::onBarDragged(const SysStaffKey& key, int rectIndex, qreal newYN, bool completed)
{
    const auto dataIt = m_overlaysByStaff.find(key);
    IF_ASSERT_FAILED(key.isValid() && dataIt != m_overlaysByStaff.end()
                     && rectIndex >= 0 && static_cast<size_t>(rectIndex) < dataIt->second.notes.size()) {
        return;
    }
    const StaffOverlayData& data = dataIt->second;

    const NoteEntry& draggedEntry = data.notes.at(rectIndex);
    Note* draggedNote = draggedEntry.note;
    IF_ASSERT_FAILED(draggedNote) {
        return;
    }

    const double canvasY = data.bandRect.y() + newYN * data.bandRect.height();
    const int newVelocity = std::clamp(velocityFromCanvasY(draggedEntry.yRange, canvasY),
                                       MIN_DRAGGABLE_VELOCITY, MAX_DRAGGABLE_VELOCITY);

    // If the dragged note is part of a multi-note selection, apply the same velocity delta to
    // every other selected note - including notes hidden behind others in the same chord's
    // stack - each clamped independently. Only what's selected moves.
    const int delta = newVelocity - displayedVelocity(draggedNote);

    std::vector<Note*> affectedNotes { draggedNote };
    if (delta != 0 || !completed) {
        const std::vector<Note*> selected = selectedNotes();
        if (selected.size() > 1 && muse::contains(selected, draggedNote)) {
            affectedNotes = selected;
        }
    }

    struct PendingChange {
        Note* note = nullptr;
        int velocity = 0;
    };
    std::vector<PendingChange> changes;
    changes.reserve(affectedNotes.size());

    for (Note* note : affectedNotes) {
        if (note == draggedNote) {
            changes.push_back({ note, newVelocity });
            continue;
        }

        const int otherVelocity = std::clamp(displayedVelocity(note) + delta, MIN_DRAGGABLE_VELOCITY, MAX_DRAGGABLE_VELOCITY);
        changes.push_back({ note, otherVelocity });
    }

    if (!completed) {
        // Live drag preview - update every affected overlay's displayed bar height without
        // touching the score.
        for (const PendingChange& change : changes) {
            const auto locIt = m_noteLocations.find(change.note);
            if (locIt != m_noteLocations.end()) {
                previewBarHeight(locIt->second, change.velocity);
            }
        }
        return;
    }

    const INotationPtr notation = currentNotation();
    const INotationUndoStackPtr undoStack = notation ? notation->undoStack() : nullptr;
    IF_ASSERT_FAILED(undoStack) {
        return;
    }

    // Dragging sets an absolute target (this overlay is a fixed 0-127 viewport), so every
    // affected note - including a VeloType::OFFSET_VAL one whose pre-drag effective value was
    // already correctly resolved via displayedVelocity() above - ends up as an absolute
    // USER_VAL. Its relative-to-the-dynamic-marking behavior is intentionally traded for "this is
    // now the value I dragged it to" once the user has directly edited it through this UI.
    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Change note velocity"));
    for (const PendingChange& change : changes) {
        if (change.note->getProperty(mu::engraving::Pid::VELO_TYPE).value<VeloType>() != VeloType::USER_VAL) {
            change.note->undoChangeProperty(mu::engraving::Pid::VELO_TYPE, VeloType::USER_VAL,
                                            mu::engraving::PropertyFlags::NOSTYLE);
        }
        change.note->undoChangeProperty(mu::engraving::Pid::USER_VELOCITY, change.velocity, mu::engraving::PropertyFlags::NOSTYLE);
    }
    undoStack->commitChanges();
}

int NotationNoteVelocityController::contextVelocity(const Note* note) const
{
    const IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();
    const INotationPlaybackPtr playback = masterNotation ? masterNotation->playback() : nullptr;
    if (!playback) {
        return 64;
    }

    const muse::mpe::dynamic_level_t level = playback->appliableDynamicLevel(note->track(), note->tick().ticks());
    const double ratio = muse::mpe::dynamicLevelToVelocityRatio(level);
    return std::clamp(static_cast<int>(std::lround(ratio * 127.0)), 0, 127);
}

int NotationNoteVelocityController::displayedVelocity(const Note* note) const
{
    const int userVelocity = note->userVelocity();
    if (userVelocity == 0) {
        return contextVelocity(note);
    }

    // Note::customizeVelocity(): VeloType::USER_VAL means userVelocity() IS the absolute value,
    // but VeloType::OFFSET_VAL means it's a *percentage* nudge applied on top of the dynamic
    // context (velo += velo * userVelocity() / 100) - treating it as absolute here would both
    // show the wrong bar height and compute a wrong drag delta for these (rare, e.g.
    // plugin-authored or imported) notes.
    const VeloType veloType = note->getProperty(mu::engraving::Pid::VELO_TYPE).value<VeloType>();
    if (veloType == VeloType::USER_VAL) {
        return userVelocity;
    }

    const int context = contextVelocity(note);
    const int offset = static_cast<int>(std::lround(context * userVelocity / 100.0));
    return std::clamp(context + offset, 0, 127);
}

INotationNoteVelocityPtr NotationNoteVelocityController::noteVelocity() const
{
    const IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();
    return masterNotation ? masterNotation->noteVelocity() : nullptr;
}

INotationPtr NotationNoteVelocityController::currentNotation() const
{
    return globalContext()->currentNotation();
}

mu::engraving::Score* NotationNoteVelocityController::score() const
{
    return currentNotation() ? currentNotation()->elements()->msScore() : nullptr;
}
