//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "noteinputpreferencesmodel.h"

#include "log.h"

using namespace mu::notation;

NoteInputPreferencesModel::NoteInputPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

bool NoteInputPreferencesModel::advanceToNextNoteOnKeyRelease() const
{
    return configuration()->advanceToNextNoteOnKeyRelease();
}

bool NoteInputPreferencesModel::colorNotesOusideOfUsablePitchRange() const
{
    return configuration()->colorNotesOusideOfUsablePitchRange();
}

int NoteInputPreferencesModel::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return configuration()->delayBetweenNotesInRealTimeModeMilliseconds();
}

bool NoteInputPreferencesModel::playNotesWhenEditing() const
{
    return configuration()->playNotesWhenEditing();
}

int NoteInputPreferencesModel::notePlayDurationMilliseconds() const
{
    return configuration()->notePlayDurationMilliseconds();
}

bool NoteInputPreferencesModel::playChordWhenEditing() const
{
    return configuration()->playChordWhenEditing();
}

bool NoteInputPreferencesModel::playChordSymbolWhenEditing() const
{
    return configuration()->playChordSymbolWhenEditing();
}

void NoteInputPreferencesModel::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    if (value == advanceToNextNoteOnKeyRelease()) {
        return;
    }

    configuration()->setAdvanceToNextNoteOnKeyRelease(value);
    emit advanceToNextNoteOnKeyReleaseChanged(value);
}

void NoteInputPreferencesModel::setColorNotesOusideOfUsablePitchRange(bool value)
{
    if (value == colorNotesOusideOfUsablePitchRange()) {
        return;
    }

    configuration()->setColorNotesOusideOfUsablePitchRange(value);
    emit colorNotesOusideOfUsablePitchRangeChanged(value);
}

void NoteInputPreferencesModel::setDelayBetweenNotesInRealTimeModeMilliseconds(int delay)
{
    if (delay == delayBetweenNotesInRealTimeModeMilliseconds()) {
        return;
    }

    configuration()->setDelayBetweenNotesInRealTimeModeMilliseconds(delay);
    emit delayBetweenNotesInRealTimeModeMillisecondsChanged(delay);
}

void NoteInputPreferencesModel::setPlayNotesWhenEditing(bool value)
{
    if (value == playNotesWhenEditing()) {
        return;
    }

    configuration()->setPlayNotesWhenEditing(value);
    emit playNotesWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setNotePlayDurationMilliseconds(int duration)
{
    if (duration == notePlayDurationMilliseconds()) {
        return;
    }

    configuration()->setNotePlayDurationMilliseconds(duration);
    emit notePlayDurationMillisecondsChanged(duration);
}

void NoteInputPreferencesModel::setPlayChordWhenEditing(bool value)
{
    if (value == playChordWhenEditing()) {
        return;
    }

    configuration()->setPlayChordWhenEditing(value);
    emit playChordWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setPlayChordSymbolWhenEditing(bool value)
{
    if (value == playChordSymbolWhenEditing()) {
        return;
    }

    configuration()->setPlayChordSymbolWhenEditing(value);
    emit playChordSymbolWhenEditingChanged(value);
}
