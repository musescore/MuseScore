/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionpanelpadmodel.h"

PercussionPanelPadModel::PercussionPanelPadModel(QObject* parent)
    : QObject(parent)
{
}

void PercussionPanelPadModel::setInstrumentName(const QString& instrumentName)
{
    if (m_instrumentName == instrumentName) {
        return;
    }

    m_instrumentName = instrumentName;
    emit instrumentNameChanged();
}

void PercussionPanelPadModel::setKeyboardShortcut(const QString& keyboardShortcut)
{
    if (m_keyboardShortcut == keyboardShortcut) {
        return;
    }

    m_keyboardShortcut = keyboardShortcut;
    emit keyboardShortcutChanged();
}

void PercussionPanelPadModel::setMidiNote(const QString& midiNote)
{
    if (m_midiNote == midiNote) {
        return;
    }

    m_midiNote = midiNote;
    emit midiNoteChanged();
}

void PercussionPanelPadModel::setIsEmptySlot(bool isEmptySlot)
{
    if (m_isEmptySlot == isEmptySlot) {
        return;
    }

    m_isEmptySlot = isEmptySlot;
    emit isEmptySlotChanged();
}
