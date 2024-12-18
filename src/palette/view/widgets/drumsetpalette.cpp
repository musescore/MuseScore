/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "drumsetpalette.h"

#include "engraving/dom/drumset.h"

#include "notation/utilities/percussionutilities.h"

#include "translation.h"
#include "log.h"

#include <QTimer>

using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::palette;

DrumsetPalette::DrumsetPalette(QWidget* parent)
    : PaletteScrollArea(nullptr, parent)
{
    setObjectName("DrumsetPalette");
    setFocusPolicy(Qt::NoFocus);

    m_drumPalette = new PaletteWidget(this);
    m_drumPalette->setMag(0.8);
    m_drumPalette->setSelectable(true);
    m_drumPalette->setUseDoubleClickForApplyingElements(true);
    m_drumPalette->setGridSize(28, 60);
    m_drumPalette->setContextMenuPolicy(Qt::PreventContextMenu);

    setWidget(m_drumPalette);
    retranslate();

    connect(m_drumPalette, &PaletteWidget::boxClicked, this, &DrumsetPalette::drumNoteClicked);
}

void DrumsetPalette::setNotation(INotationPtr notation)
{
    m_notation = notation;
}

void DrumsetPalette::retranslate()
{
    m_drumPalette->setName(muse::qtrc("palette", "Drumset"));
}

void DrumsetPalette::updateDrumset()
{
    INotationNoteInputPtr noteInput = this->noteInput();
    if (!noteInput) {
        clear();
        return;
    }

    NoteInputState state = noteInput->state();
    if (m_drumset == state.drumset) {
        return;
    }

    clear();
    m_drumset = state.drumset;

    if (!m_drumset) {
        return;
    }

    TRACEFUNC;

    for (int pitch = 0; pitch < engraving::DRUM_INSTRUMENTS; ++pitch) {
        if (!m_drumset->isValid(pitch)) {
            continue;
        }

        std::shared_ptr<Chord> chord = notation::PercussionUtilities::getDrumNoteForPreview(m_drumset, pitch);

        int shortcutCode = m_drumset->shortcut(pitch);
        QString shortcut = shortcutCode != 0 ? QChar(shortcutCode) : QString();

        m_drumPalette->appendElement(chord, m_drumset->translatedName(pitch), 1.0, QPointF(0, 0), shortcut);
    }

    noteInput->setDrumNote(selectedDrumNote());
}

void DrumsetPalette::clear()
{
    m_drumset = nullptr;
    m_drumPalette->clear();
}

void DrumsetPalette::drumNoteClicked(int val)
{
    INotationNoteInputPtr noteInput = this->noteInput();
    if (!noteInput) {
        return;
    }

    ElementPtr element = m_drumPalette->elementForCellAt(val);
    if (!element || element->type() != ElementType::CHORD) {
        return;
    }

    const Chord* ch = mu::engraving::toChord(element.get());
    bool newChordSelected = val != m_drumPalette->selectedIdx();

    if (newChordSelected) {
        const Note* note = ch->downNote();

        track_idx_t track = (noteInput->state().currentTrack / mu::engraving::VOICES) * mu::engraving::VOICES + element->track();

        noteInput->setCurrentTrack(track);
        noteInput->setDrumNote(note->pitch());

        PaletteCellPtr pitchCell = m_drumPalette->cellAt(val);
        m_pitchNameChanged.send(pitchCell->name);
    }

    previewSound(ch, newChordSelected, noteInput->state());
}

void DrumsetPalette::previewSound(const Chord* chord, bool newChordSelected, const notation::NoteInputState& inputState)
{
    //! NOTE: prevents "sound spam" after multiple clicks on a selected element
    static QTimer soundPauseTimer;
    if (soundPauseTimer.isActive() && !newChordSelected) {
        return;
    }

    Chord* preview = chord->clone();
    preview->setParent(inputState.segment);
    preview->setTrack(inputState.currentTrack);

    const std::vector<Note*>& previewNotes = preview->notes();
    const std::vector<Note*>& chordNotes = chord->notes();
    IF_ASSERT_FAILED(previewNotes.size() == chordNotes.size()) {
        return;
    }

    for (size_t i = 0; i < previewNotes.size(); ++i) {
        SymId symId = chordNotes.at(i)->ldata()->cachedNoteheadSym.value();
        previewNotes.at(i)->mutldata()->cachedNoteheadSym.set_value(symId);
    }

    playback()->playElements({ preview });

    delete preview;

    soundPauseTimer.setSingleShot(true);
    soundPauseTimer.start(200);
}

int DrumsetPalette::selectedDrumNote()
{
    int idx = m_drumPalette->selectedIdx();
    if (idx < 0) {
        return -1;
    }

    ElementPtr element = m_drumPalette->elementForCellAt(idx);
    if (element && element->type() == ElementType::CHORD) {
        const Chord* ch = dynamic_cast<Chord*>(element.get());
        const Note* note = ch->downNote();
        auto pitchCell = m_drumPalette->cellAt(idx);
        m_pitchNameChanged.send(pitchCell->name);
        return note->pitch();
    }

    return -1;
}

void DrumsetPalette::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}

void DrumsetPalette::mousePressEvent(QMouseEvent* event)
{
    m_drumPalette->handleEvent(event);
}

void DrumsetPalette::mouseDoubleClickEvent(QMouseEvent* event)
{
    m_drumPalette->handleEvent(event);
}

void DrumsetPalette::mouseMoveEvent(QMouseEvent* event)
{
    m_drumPalette->handleEvent(event);
}

void DrumsetPalette::enterEvent(QEnterEvent* event)
{
    m_drumPalette->handleEvent(event);
}

void DrumsetPalette::leaveEvent(QEvent* event)
{
    m_drumPalette->handleEvent(event);
}

bool DrumsetPalette::handleEvent(QEvent* event)
{
    return QWidget::event(event);
}

muse::async::Channel<QString> DrumsetPalette::pitchNameChanged() const
{
    return m_pitchNameChanged;
}

PaletteWidget* DrumsetPalette::paletteWidget() const
{
    return m_drumPalette;
}

INotationNoteInputPtr DrumsetPalette::noteInput() const
{
    return m_notation ? m_notation->interaction()->noteInput() : nullptr;
}
