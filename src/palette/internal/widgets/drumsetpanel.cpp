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

#include "drumsetpanel.h"

#include "translation.h"
#include "log.h"

#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/drumset.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/stem.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"

namespace Ms {
DrumsetPanel::DrumsetPanel(QWidget* parent)
    : PaletteScrollArea(nullptr, parent)
{
    setObjectName("DrumsetPanel");
    setFocusPolicy(Qt::NoFocus);

    drumPalette = new Palette(this);
    drumPalette->setMag(0.8);
    drumPalette->setSelectable(true);
    drumPalette->setUseDoubleClickToActivate(true);
    drumPalette->setGrid(28, 60);
    drumPalette->setContextMenuPolicy(Qt::PreventContextMenu);

    setWidget(drumPalette);
    retranslate();

    connect(drumPalette, SIGNAL(boxClicked(int)), SLOT(drumNoteSelected(int)));
}

void DrumsetPanel::setNotation(mu::notation::INotationPtr notation)
{
    m_notation = notation;
    updateDrumset();
}

Score* DrumsetPanel::score() const
{
    return m_notation ? m_notation->elements()->msScore() : nullptr;
}

void DrumsetPanel::retranslate()
{
    drumPalette->setName(mu::qtrc("palette", "Drumset"));
}

void DrumsetPanel::updateDrumset()
{
    Score* score = this->score();

    if (!score) {
        return;
    }

    drumPalette->clear();

    const InputState& inputState = score->inputState();
    drumset = inputState.drumset();
    staff = score->staff(inputState.track() / VOICES);

    if (!drumset) {
        return;
    }

    double _spatium = gscore->spatium();

    for (int pitch = 0; pitch < 128; ++pitch) {
        if (!drumset->isValid(pitch)) {
            continue;
        }

        bool up;
        int line      = drumset->line(pitch);
        NoteHead::Group noteHead  = drumset->noteHead(pitch);
        int voice     = drumset->voice(pitch);
        Direction dir = drumset->stemDirection(pitch);
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
            noteheadSym = drumset->noteHeads(pitch, NoteHead::Type::HEAD_QUARTER);
        } else {
            noteheadSym = note->noteHead(true, noteHead, NoteHead::Type::HEAD_QUARTER);
        }

        note->setCachedNoteheadSym(noteheadSym);     // we use the cached notehead so we don't recompute it at each layout
        chord->add(note);
        int sc = drumset->shortcut(pitch);
        QString shortcut;
        if (sc) {
            shortcut = QChar(sc);
        }
        drumPalette->append(chord, mu::qtrc("drumset", drumset->name(pitch).toUtf8().data()), shortcut);
    }
}

void DrumsetPanel::drumNoteSelected(int val)
{
    Score* score = this->score();

    if (!score) {
        return;
    }

    ElementPtr element = drumPalette->element(val);
    if (!element || element->type() != ElementType::CHORD) {
        return;
    }

    const Chord* ch = dynamic_cast<Chord*>(element.get());
    const Note* note = ch->downNote();
    int pitch = note->pitch();

    int track = (score->inputState().track() / VOICES) * VOICES + element->track();
    score->inputState().setTrack(track);
    score->inputState().setDrumNote(pitch);
    m_notation->interaction()->noteInput()->stateChanged().notify();

    QString voiceActionCode = QString("voice-%1").arg(element->voice() + 1);
    dispatcher()->dispatch(voiceActionCode.toStdString());

    auto pitchCell = drumPalette->cellAt(val);
    m_pitchNameChanged.send(pitchCell->name);
}

int DrumsetPanel::selectedDrumNote()
{
    int idx = drumPalette->getSelectedIdx();
    if (idx < 0) {
        return -1;
    }

    ElementPtr element = drumPalette->element(idx);
    if (element && element->type() == ElementType::CHORD) {
        const Chord* ch = dynamic_cast<Chord*>(element.get());
        const Note* note = ch->downNote();
        auto pitchCell = drumPalette->cellAt(idx);
        m_pitchNameChanged.send(pitchCell->name);
        return note->pitch();
    }

    return -1;
}

void DrumsetPanel::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}

void DrumsetPanel::mousePressEvent(QMouseEvent* event)
{
    drumPalette->handleEvent(event);
}

void DrumsetPanel::mouseMoveEvent(QMouseEvent* event)
{
    drumPalette->handleEvent(event);
}

bool DrumsetPanel::handleEvent(QEvent* e)
{
    return QWidget::event(e);
}

mu::async::Channel<QString> DrumsetPanel::pitchNameChanged() const
{
    return m_pitchNameChanged;
}
}
