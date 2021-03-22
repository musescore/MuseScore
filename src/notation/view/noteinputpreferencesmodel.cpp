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
    NOT_IMPLEMENTED;
    return false;
}

bool NoteInputPreferencesModel::colorNotesOusideOfUsablePitchRange() const
{
    NOT_IMPLEMENTED;
    return false;
}

int NoteInputPreferencesModel::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    NOT_IMPLEMENTED;
    return 0;
}

bool NoteInputPreferencesModel::playNotesWhenEditing() const
{
    NOT_IMPLEMENTED;
    return true;
}

int NoteInputPreferencesModel::notePlayDurationMilliseconds() const
{
    NOT_IMPLEMENTED;
    return 0;
}

bool NoteInputPreferencesModel::playChordWhenEditing() const
{
    NOT_IMPLEMENTED;
    return false;
}

bool NoteInputPreferencesModel::playChordSymbolWhenEditing() const
{
    NOT_IMPLEMENTED;
    return false;
}

void NoteInputPreferencesModel::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    NOT_IMPLEMENTED;

    if (value == advanceToNextNoteOnKeyRelease()) {
        return;
    }

    emit advanceToNextNoteOnKeyReleaseChanged(value);
}

void NoteInputPreferencesModel::setColorNotesOusideOfUsablePitchRange(bool value)
{
    NOT_IMPLEMENTED;

    if (value == colorNotesOusideOfUsablePitchRange()) {
        return;
    }

    emit colorNotesOusideOfUsablePitchRangeChanged(value);
}

void NoteInputPreferencesModel::setDelayBetweenNotesInRealTimeModeMilliseconds(int delay)
{
    NOT_IMPLEMENTED;

    if (delay == delayBetweenNotesInRealTimeModeMilliseconds()) {
        return;
    }

    emit delayBetweenNotesInRealTimeModeMillisecondsChanged(delay);
}

void NoteInputPreferencesModel::setPlayNotesWhenEditing(bool value)
{
    NOT_IMPLEMENTED;

    if (value == playNotesWhenEditing()) {
        return;
    }

    emit playNotesWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setNotePlayDurationMilliseconds(int duration)
{
    NOT_IMPLEMENTED;

    if (duration == notePlayDurationMilliseconds()) {
        return;
    }

    emit notePlayDurationMillisecondsChanged(duration);
}

void NoteInputPreferencesModel::setPlayChordWhenEditing(bool value)
{
    NOT_IMPLEMENTED;

    if (value == playChordWhenEditing()) {
        return;
    }

    emit playChordWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setPlayChordSymbolWhenEditing(bool value)
{
    NOT_IMPLEMENTED;

    if (value == playChordSymbolWhenEditing()) {
        return;
    }

    emit playChordSymbolWhenEditingChanged(value);
}
