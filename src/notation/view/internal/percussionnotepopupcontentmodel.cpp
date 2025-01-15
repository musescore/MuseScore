/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "percussionnotepopupcontentmodel.h"

#include "engraving/dom/drumset.h"
#include "engraving/dom/shadownote.h"

using namespace mu::notation;

PercussionNotePopupContentModel::PercussionNotePopupContentModel(QObject* parent)
    : QObject{parent}
{
}

void PercussionNotePopupContentModel::init()
{
    interaction()->shadowNoteChanged().onNotify(this, [this]() {
        emit shouldShowButtonsChanged();
        emit percussionNoteNameChanged();
        emit keyboardShortcutChanged();
    });
}

void PercussionNotePopupContentModel::prevDrumNote()
{
    const Drumset* ds = currentDrumset();
    mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    IF_ASSERT_FAILED(ds && shadowNote) {
        return;
    }

    const int currNote = shadowNote->drumNotePitch();
    const int currLine = ds->line(currNote);

    int pitch = currNote - 1;
    while (pitch != currNote) {
        if (pitch < 0) {
            // Wrap around
            pitch = mu::engraving::DRUM_INSTRUMENTS - 1;
        }
        if (ds->isValid(pitch) && ds->line(pitch) == currLine) {
            shadowNote->setDrumNotePitch(pitch);
            interaction()->showShadowNote(shadowNote->pos());
            interaction()->selectionChanged().notify(); // TEMPORARY HACK - triggers AbstractNotationPaintView::scheduleRedraw
            emit percussionNoteNameChanged();
            emit keyboardShortcutChanged();
            return;
        }
        --pitch;
    }

    UNREACHABLE;
}

void PercussionNotePopupContentModel::nextDrumNote()
{
    const Drumset* ds = currentDrumset();
    mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    IF_ASSERT_FAILED(ds && shadowNote) {
        return;
    }

    const int currNote = shadowNote->drumNotePitch();
    const int currLine = ds->line(currNote);

    int pitch = currNote + 1;
    while (pitch != currNote) {
        if (pitch == mu::engraving::DRUM_INSTRUMENTS) {
            // Wrap around
            pitch = 0;
        }
        if (ds->isValid(pitch) && ds->line(pitch) == currLine) {
            shadowNote->setDrumNotePitch(pitch);
            interaction()->showShadowNote(shadowNote->pos());
            interaction()->selectionChanged().notify(); // TEMPORARY HACK - triggers AbstractNotationPaintView::scheduleRedraw
            emit percussionNoteNameChanged();
            emit keyboardShortcutChanged();
            return;
        }
        ++pitch;
    }

    UNREACHABLE;
}

bool PercussionNotePopupContentModel::shouldShowButtons() const
{
    const Drumset* ds = currentDrumset();
    const mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    if (!ds || !shadowNote || shadowNote->drumNotePitch() < 0) {
        return false;
    }

    const int line = ds->line(shadowNote->drumNotePitch());

    int drumsOnSameLine = 0;
    for (int pitch = 0; pitch < mu::engraving::DRUM_INSTRUMENTS; ++pitch) {
        if (ds->isValid(pitch) && ds->line(pitch) == line) {
            ++drumsOnSameLine;
        }
        if (drumsOnSameLine > 1) {
            return true;
        }
    }

    return false;
}

QString PercussionNotePopupContentModel::percussionNoteName() const
{
    const Drumset* ds = currentDrumset();
    const mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    if (!ds || !shadowNote || shadowNote->drumNotePitch() < 0) {
        return QString();
    }

    return ds->name(shadowNote->drumNotePitch());
}

QString PercussionNotePopupContentModel::keyboardShortcut() const
{
    const Drumset* ds = currentDrumset();
    const mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    if (!ds || !shadowNote || shadowNote->drumNotePitch() < 0) {
        return QString();
    }

    const int shortcut = ds->shortcut(shadowNote->drumNotePitch());
    return shortcut ? QString("(%1)").arg(QChar(shortcut)) : QString();
}

INotationInteractionPtr PercussionNotePopupContentModel::interaction() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

mu::engraving::ShadowNote* PercussionNotePopupContentModel::currentShadowNote() const
{
    return interaction() ? interaction()->shadowNote() : nullptr;
}

const Drumset* PercussionNotePopupContentModel::currentDrumset() const
{
    const mu::engraving::ShadowNote* shadowNote = currentShadowNote();
    if (!shadowNote) {
        return nullptr;
    }

    const Part* part = shadowNote->part();
    if (!part) {
        return nullptr;
    }

    const Instrument* inst = part->instrument(shadowNote->tick());
    return inst ? inst->drumset() : nullptr;
}
