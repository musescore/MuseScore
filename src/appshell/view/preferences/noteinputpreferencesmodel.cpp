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

#include "noteinputpreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;

NoteInputPreferencesModel::NoteInputPreferencesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void NoteInputPreferencesModel::load()
{
    notationConfiguration()->defaultNoteInputMethodChanged().onNotify(this, [this]() {
        emit defaultNoteInputMethodChanged(defaultNoteInputMethod());
    });

    notationConfiguration()->addAccidentalDotsArticulationsToNextNoteEnteredChanged().onNotify(this, [this]() {
        emit addAccidentalDotsArticulationsToNextNoteEnteredChanged(addAccidentalDotsArticulationsToNextNoteEntered());
    });

    notationConfiguration()->useNoteInputCursorInInputByDurationChanged().onNotify(this, [this]() {
        emit useNoteInputCursorInInputByDurationChanged(useNoteInputCursorInInputByDuration());
    });

    notationConfiguration()->isMidiInputEnabledChanged().onNotify(this, [this]() {
        emit midiInputEnabledChanged(midiInputEnabled());
    });

    notationConfiguration()->startNoteInputAtSelectionWhenPressingMidiKeyChanged().onNotify(this, [this]() {
        emit startNoteInputAtSelectionWhenPressingMidiKeyChanged(startNoteInputAtSelectionWhenPressingMidiKey());
    });

    playbackConfiguration()->playNotesWhenEditingChanged().onNotify(this, [this]() {
        emit playNotesWhenEditingChanged(playNotesWhenEditing());
    });

    notationConfiguration()->isPlayPreviewNotesInInputByDurationChanged().onNotify(this, [this]() {
        emit playPreviewNotesInInputByDurationChanged(playPreviewNotesInInputByDuration());
    });

    shortcutsConfiguration()->advanceToNextNoteOnKeyReleaseChanged().onReceive(this, [this](bool value) {
        emit advanceToNextNoteOnKeyReleaseChanged(value);
    });

    notationConfiguration()->delayBetweenNotesInRealTimeModeMillisecondsChanged().onReceive(this, [this](int value) {
        emit delayBetweenNotesInRealTimeModeMillisecondsChanged(value);
    });

    notationConfiguration()->notePlayDurationMillisecondsChanged().onReceive(this, [this](int value) {
        emit notePlayDurationMillisecondsChanged(value);
    });

    playbackConfiguration()->playChordWhenEditingChanged().onReceive(this, [this](bool value) {
        emit playChordWhenEditingChanged(value);
    });

    playbackConfiguration()->playHarmonyWhenEditingChanged().onReceive(this, [this](bool value) {
        emit playChordSymbolWhenEditingChanged(value);
    });

    engravingConfiguration()->dynamicsApplyToAllVoicesChanged().onReceive(this, [this](bool value) {
        emit dynamicsApplyToAllVoicesChanged(value);
    });

    notationConfiguration()->colorNotesOutsideOfUsablePitchRangeChanged().onReceive(this, [this](bool value) {
        emit colorNotesOutsideOfUsablePitchRangeChanged(value);
    });

    notationConfiguration()->warnGuitarBendsChanged().onReceive(this, [this](bool value) {
        emit warnGuitarBendsChanged(value);
    });
}

QVariantList NoteInputPreferencesModel::noteInputMethods() const
{
    using Method = mu::notation::NoteInputMethod;

    std::vector<std::pair<muse::actions::ActionCode, Method > > noteInputActions {
        { "note-input-by-note-name", Method::BY_NOTE_NAME },
        { "note-input-by-duration", Method::BY_DURATION },
    };

    QVariantList methods;

    for (const auto& pair : noteInputActions) {
        const muse::ui::UiAction& action = uiActionsRegister()->action(pair.first);

        QVariantMap method;
        method["value"] = static_cast<int>(pair.second);
        method["text"] = action.title.qTranslatedWithoutMnemonic();

        methods.emplace_back(std::move(method));
    }

    return methods;
}

int NoteInputPreferencesModel::defaultNoteInputMethod() const
{
    return static_cast<int>(notationConfiguration()->defaultNoteInputMethod());
}

bool NoteInputPreferencesModel::addAccidentalDotsArticulationsToNextNoteEntered() const
{
    return notationConfiguration()->addAccidentalDotsArticulationsToNextNoteEntered();
}

bool NoteInputPreferencesModel::useNoteInputCursorInInputByDuration() const
{
    return notationConfiguration()->useNoteInputCursorInInputByDuration();
}

bool NoteInputPreferencesModel::midiInputEnabled() const
{
    return notationConfiguration()->isMidiInputEnabled();
}

bool NoteInputPreferencesModel::startNoteInputAtSelectionWhenPressingMidiKey() const
{
    return notationConfiguration()->startNoteInputAtSelectionWhenPressingMidiKey();
}

bool NoteInputPreferencesModel::advanceToNextNoteOnKeyRelease() const
{
    return shortcutsConfiguration()->advanceToNextNoteOnKeyRelease();
}

int NoteInputPreferencesModel::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return notationConfiguration()->delayBetweenNotesInRealTimeModeMilliseconds();
}

bool NoteInputPreferencesModel::playNotesWhenEditing() const
{
    return playbackConfiguration()->playNotesWhenEditing();
}

bool NoteInputPreferencesModel::playPreviewNotesInInputByDuration() const
{
    return notationConfiguration()->isPlayPreviewNotesInInputByDuration();
}

int NoteInputPreferencesModel::notePlayDurationMilliseconds() const
{
    return notationConfiguration()->notePlayDurationMilliseconds();
}

bool NoteInputPreferencesModel::playChordWhenEditing() const
{
    return playbackConfiguration()->playChordWhenEditing();
}

bool NoteInputPreferencesModel::playChordSymbolWhenEditing() const
{
    return playbackConfiguration()->playHarmonyWhenEditing();
}

bool NoteInputPreferencesModel::playNotesOnMidiInput() const
{
    return playbackConfiguration()->playNotesOnMidiInput();
}

bool NoteInputPreferencesModel::dynamicsApplyToAllVoices() const
{
    return engravingConfiguration()->dynamicsApplyToAllVoices();
}

bool NoteInputPreferencesModel::colorNotesOutsideOfUsablePitchRange() const
{
    return notationConfiguration()->colorNotesOutsideOfUsablePitchRange();
}

bool NoteInputPreferencesModel::warnGuitarBends() const
{
    return notationConfiguration()->warnGuitarBends();
}

void NoteInputPreferencesModel::setDefaultNoteInputMethod(int value)
{
    if (value == defaultNoteInputMethod()) {
        return;
    }

    notationConfiguration()->setDefaultNoteInputMethod(static_cast<mu::notation::NoteInputMethod>(value));
    emit defaultNoteInputMethodChanged(value);
}

void NoteInputPreferencesModel::setAddAccidentalDotsArticulationsToNextNoteEntered(bool value)
{
    if (value == addAccidentalDotsArticulationsToNextNoteEntered()) {
        return;
    }

    notationConfiguration()->setAddAccidentalDotsArticulationsToNextNoteEntered(value);
    emit addAccidentalDotsArticulationsToNextNoteEnteredChanged(value);
}

void NoteInputPreferencesModel::setUseNoteInputCursorInInputByDuration(bool value)
{
    if (value == useNoteInputCursorInInputByDuration()) {
        return;
    }

    notationConfiguration()->setUseNoteInputCursorInInputByDuration(value);
    emit useNoteInputCursorInInputByDurationChanged(value);
}

void NoteInputPreferencesModel::setMidiInputEnabled(bool value)
{
    if (value == midiInputEnabled()) {
        return;
    }

    notationConfiguration()->setIsMidiInputEnabled(value);
    emit midiInputEnabledChanged(value);
}

void NoteInputPreferencesModel::setStartNoteInputAtSelectionWhenPressingMidiKey(bool value)
{
    if (value == startNoteInputAtSelectionWhenPressingMidiKey()) {
        return;
    }

    notationConfiguration()->setStartNoteInputAtSelectionWhenPressingMidiKey(value);
    emit startNoteInputAtSelectionWhenPressingMidiKeyChanged(value);
}

void NoteInputPreferencesModel::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    if (value == advanceToNextNoteOnKeyRelease()) {
        return;
    }

    shortcutsConfiguration()->setAdvanceToNextNoteOnKeyRelease(value);
    emit advanceToNextNoteOnKeyReleaseChanged(value);
}

void NoteInputPreferencesModel::setDelayBetweenNotesInRealTimeModeMilliseconds(int delay)
{
    if (delay == delayBetweenNotesInRealTimeModeMilliseconds()) {
        return;
    }

    notationConfiguration()->setDelayBetweenNotesInRealTimeModeMilliseconds(delay);
    emit delayBetweenNotesInRealTimeModeMillisecondsChanged(delay);
}

void NoteInputPreferencesModel::setPlayNotesWhenEditing(bool value)
{
    if (value == playNotesWhenEditing()) {
        return;
    }

    playbackConfiguration()->setPlayNotesWhenEditing(value);
    emit playNotesWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setPlayPreviewNotesInInputByDuration(bool value)
{
    if (value == playPreviewNotesInInputByDuration()) {
        return;
    }

    notationConfiguration()->setIsPlayPreviewNotesInInputByDuration(value);
    emit playPreviewNotesInInputByDurationChanged(value);
}

void NoteInputPreferencesModel::setNotePlayDurationMilliseconds(int duration)
{
    if (duration == notePlayDurationMilliseconds()) {
        return;
    }

    notationConfiguration()->setNotePlayDurationMilliseconds(duration);
    emit notePlayDurationMillisecondsChanged(duration);
}

void NoteInputPreferencesModel::setPlayChordWhenEditing(bool value)
{
    if (value == playChordWhenEditing()) {
        return;
    }

    playbackConfiguration()->setPlayChordWhenEditing(value);
    emit playChordWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setPlayChordSymbolWhenEditing(bool value)
{
    if (value == playChordSymbolWhenEditing()) {
        return;
    }

    playbackConfiguration()->setPlayHarmonyWhenEditing(value);
    emit playChordSymbolWhenEditingChanged(value);
}

void NoteInputPreferencesModel::setPlayNotesOnMidiInput(bool value)
{
    if (value == playNotesOnMidiInput()) {
        return;
    }

    playbackConfiguration()->setPlayNotesOnMidiInput(value);
    emit playNotesOnMidiInputChanged(value);
}

void NoteInputPreferencesModel::setDynamicsApplyToAllVoices(bool value)
{
    if (value == dynamicsApplyToAllVoices()) {
        return;
    }

    engravingConfiguration()->setDynamicsApplyToAllVoices(value);
    emit dynamicsApplyToAllVoicesChanged(value);
}

void NoteInputPreferencesModel::setColorNotesOutsideOfUsablePitchRange(bool value)
{
    if (value == colorNotesOutsideOfUsablePitchRange()) {
        return;
    }

    notationConfiguration()->setColorNotesOutsideOfUsablePitchRange(value);
    emit colorNotesOutsideOfUsablePitchRangeChanged(value);
}

void NoteInputPreferencesModel::setWarnGuitarBends(bool value)
{
    if (value == warnGuitarBends()) {
        return;
    }

    notationConfiguration()->setWarnGuitarBends(value);
    emit warnGuitarBendsChanged(value);
}
