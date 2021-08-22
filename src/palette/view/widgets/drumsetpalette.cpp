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

#include "translation.h"
#include "log.h"

#include "engraving/libmscore/chord.h"
#include "engraving/libmscore/note.h"
#include "engraving/libmscore/drumset.h"
#include "engraving/libmscore/score.h"
#include "engraving/libmscore/staff.h"
#include "engraving/libmscore/part.h"
#include "engraving/libmscore/stem.h"
#include "engraving/libmscore/mscore.h"
#include "engraving/libmscore/undo.h"

using namespace mu::notation;
using namespace mu::palette;
using namespace Ms;

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

    connect(m_drumPalette, SIGNAL(boxClicked(int)), SLOT(drumNoteSelected(int)));
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

    double _spatium = gscore->spatium();

    for (int pitch = 0; pitch < 128; ++pitch) {
        if (!m_drumset->isValid(pitch)) {
            continue;
        }

        bool up = false;
        int line = m_drumset->line(pitch);
        NoteHead::Group noteHead = m_drumset->noteHead(pitch);
        int voice = m_drumset->voice(pitch);
        Direction dir = m_drumset->stemDirection(pitch);

        if (dir == Direction::UP) {
            up = true;
        } else if (dir == Direction::DOWN) {
            up = false;
        } else {
            up = line > 4;
        }

        auto chord = std::make_shared<Chord>(gscore);
        chord->setDurationType(TDuration::DurationType::V_QUARTER);
        chord->setStemDirection(dir);
        chord->setUp(up);
        chord->setTrack(voice);
        Stem* stem = new Stem(gscore);
        stem->setLen((up ? -3.0 : 3.0) * _spatium);
        chord->add(stem);
        Note* note = new Note(gscore);
        note->setMark(true);
        note->setParent(chord.get());
        note->setTrack(voice);
        note->setPitch(pitch);
        note->setTpcFromPitch();
        note->setLine(line);
        note->setPos(0.0, _spatium * .5 * line);
        note->setHeadGroup(noteHead);
        SymId noteheadSym = SymId::noteheadBlack;
        if (noteHead == NoteHead::Group::HEAD_CUSTOM) {
            noteheadSym = m_drumset->noteHeads(pitch, NoteHead::Type::HEAD_QUARTER);
        } else {
            noteheadSym = note->noteHead(true, noteHead, NoteHead::Type::HEAD_QUARTER);
        }

        note->setCachedNoteheadSym(noteheadSym);     // we use the cached notehead so we don't recompute it at each layout
        chord->add(note);
        m_drumPalette->appendElement(chord, mu::qtrc("drumset", m_drumset->name(pitch).toUtf8().data()));
    }

    noteInput->setDrumNote(selectedDrumNote());
}

void DrumsetPalette::clear()
{
    m_drumset = nullptr;
    m_drumPalette->clear();
}

void DrumsetPalette::drumNoteSelected(int val)
{
    INotationNoteInputPtr noteInput = this->noteInput();
    if (!noteInput) {
        return;
    }

    ElementPtr element = m_drumPalette->elementForCellAt(val);
    if (!element || element->type() != ElementType::CHORD) {
        return;
    }

    TRACEFUNC;

    const Chord* ch = dynamic_cast<Chord*>(element.get());
    const Note* note = ch->downNote();
    int pitch = note->pitch();
    int voice = element->voice();

    noteInput->setCurrentVoiceIndex(voice);
    noteInput->setDrumNote(pitch);

    QString voiceActionCode = QString("voice-%1").arg(voice + 1);
    dispatcher()->dispatch(voiceActionCode.toStdString());

    auto pitchCell = m_drumPalette->cellAt(val);
    m_pitchNameChanged.send(pitchCell->name);
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

INotationNoteInputPtr DrumsetPalette::noteInput() const
{
    return m_notation ? m_notation->interaction()->noteInput() : nullptr;
}
