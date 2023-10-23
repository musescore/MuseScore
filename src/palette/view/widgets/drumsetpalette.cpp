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

#include "drumsetpalette.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/mscore.h"

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
    m_drumPalette->setName(mu::qtrc("palette", "Drumset"));
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

    double _spatium = gpaletteScore->style().spatium();

    for (int pitch = 0; pitch < 128; ++pitch) {
        if (!m_drumset->isValid(pitch)) {
            continue;
        }

        bool up = false;
        int line = m_drumset->line(pitch);
        NoteHeadGroup noteHead = m_drumset->noteHead(pitch);
        int voice = m_drumset->voice(pitch);
        DirectionV dir = m_drumset->stemDirection(pitch);

        if (dir == DirectionV::UP) {
            up = true;
        } else if (dir == DirectionV::DOWN) {
            up = false;
        } else {
            up = line > 4;
        }

        auto chord = Factory::makeChord(gpaletteScore->dummy()->segment());
        chord->setDurationType(DurationType::V_QUARTER);
        chord->setStemDirection(dir);
        chord->setIsUiItem(true);
        chord->setTrack(voice);
        Note* note = Factory::createNote(chord.get());
        note->setMark(true);
        note->setParent(chord.get());
        note->setTrack(voice);
        note->setPitch(pitch);
        note->setTpcFromPitch();
        note->setLine(line);
        note->setPos(0.0, _spatium * .5 * line);
        note->setHeadGroup(noteHead);
        SymId noteheadSym = SymId::noteheadBlack;
        if (noteHead == NoteHeadGroup::HEAD_CUSTOM) {
            noteheadSym = m_drumset->noteHeads(pitch, NoteHeadType::HEAD_QUARTER);
        } else {
            noteheadSym = note->noteHead(true, noteHead, NoteHeadType::HEAD_QUARTER);
        }

        note->mutldata()->setCachedNoteheadSym(noteheadSym);     // we use the cached notehead so we don't recompute it at each layout
        chord->add(note);

        Stem* stem = Factory::createStem(chord.get());
        stem->setParent(chord.get());
        stem->setBaseLength(Millimetre((up ? -3.0 : 3.0) * _spatium));
        chord->add(stem);

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

void DrumsetPalette::enterEvent(QEvent* event)
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

mu::async::Channel<QString> DrumsetPalette::pitchNameChanged() const
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
