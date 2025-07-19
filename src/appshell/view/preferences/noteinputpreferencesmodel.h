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
#ifndef MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H
#define MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "engraving/iengravingconfiguration.h"
#include "shortcuts/ishortcutsconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "playback/iplaybackconfiguration.h"
#include "ui/iuiactionsregister.h"

namespace mu::appshell {
class NoteInputPreferencesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(
        int defaultNoteInputMethod READ defaultNoteInputMethod WRITE setDefaultNoteInputMethod NOTIFY defaultNoteInputMethodChanged)
    Q_PROPERTY(
        bool addAccidentalDotsArticulationsToNextNoteEntered READ addAccidentalDotsArticulationsToNextNoteEntered WRITE setAddAccidentalDotsArticulationsToNextNoteEntered NOTIFY addAccidentalDotsArticulationsToNextNoteEnteredChanged)
    Q_PROPERTY(
        bool useNoteInputCursorInInputByDuration READ useNoteInputCursorInInputByDuration WRITE setUseNoteInputCursorInInputByDuration NOTIFY useNoteInputCursorInInputByDurationChanged)

    Q_PROPERTY(bool midiInputEnabled READ midiInputEnabled WRITE setMidiInputEnabled NOTIFY midiInputEnabledChanged)
    Q_PROPERTY(
        bool startNoteInputAtSelectedNoteRestWhenPressingMidiKey READ startNoteInputAtSelectedNoteRestWhenPressingMidiKey WRITE setStartNoteInputAtSelectedNoteRestWhenPressingMidiKey NOTIFY startNoteInputAtSelectedNoteRestWhenPressingMidiKeyChanged)
    Q_PROPERTY(
        bool advanceToNextNoteOnKeyRelease READ advanceToNextNoteOnKeyRelease WRITE setAdvanceToNextNoteOnKeyRelease NOTIFY advanceToNextNoteOnKeyReleaseChanged)
    Q_PROPERTY(
        bool colorNotesOutsideOfUsablePitchRange READ colorNotesOutsideOfUsablePitchRange WRITE setColorNotesOutsideOfUsablePitchRange NOTIFY colorNotesOutsideOfUsablePitchRangeChanged)
    Q_PROPERTY(
        bool warnGuitarBends READ warnGuitarBends WRITE setWarnGuitarBends NOTIFY warnGuitarBendsChanged)
    Q_PROPERTY(
        int delayBetweenNotesInRealTimeModeMilliseconds READ delayBetweenNotesInRealTimeModeMilliseconds WRITE setDelayBetweenNotesInRealTimeModeMilliseconds NOTIFY delayBetweenNotesInRealTimeModeMillisecondsChanged)

    Q_PROPERTY(bool playNotesWhenEditing READ playNotesWhenEditing WRITE setPlayNotesWhenEditing NOTIFY playNotesWhenEditingChanged)
    Q_PROPERTY(
        bool playPreviewNotesInInputByDuration READ playPreviewNotesInInputByDuration WRITE setPlayPreviewNotesInInputByDuration NOTIFY playPreviewNotesInInputByDurationChanged)
    Q_PROPERTY(
        int notePlayDurationMilliseconds READ notePlayDurationMilliseconds WRITE setNotePlayDurationMilliseconds NOTIFY notePlayDurationMillisecondsChanged)
    Q_PROPERTY(bool playChordWhenEditing READ playChordWhenEditing WRITE setPlayChordWhenEditing NOTIFY playChordWhenEditingChanged)
    Q_PROPERTY(
        bool playChordSymbolWhenEditing READ playChordSymbolWhenEditing WRITE setPlayChordSymbolWhenEditing NOTIFY playChordSymbolWhenEditingChanged)
    Q_PROPERTY(
        bool playPreviewNotesWithScoreDynamics READ playPreviewNotesWithScoreDynamics WRITE setPlayPreviewNotesWithScoreDynamics NOTIFY playPreviewNotesWithScoreDynamicsChanged)
    Q_PROPERTY(bool playNotesOnMidiInput READ playNotesOnMidiInput WRITE setPlayNotesOnMidiInput NOTIFY playNotesOnMidiInputChanged)

    Q_PROPERTY(
        bool dynamicsApplyToAllVoices READ dynamicsApplyToAllVoices WRITE setDynamicsApplyToAllVoices NOTIFY dynamicsApplyToAllVoicesChanged FINAL)

    Q_PROPERTY(
        bool autoUpdateFretboardDiagrams READ autoUpdateFretboardDiagrams WRITE setAutoUpdateFretboardDiagrams NOTIFY autoUpdateFretboardDiagramsChanged FINAL)

    muse::Inject<muse::shortcuts::IShortcutsConfiguration> shortcutsConfiguration = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<playback::IPlaybackConfiguration> playbackConfiguration = { this };
    muse::Inject<mu::engraving::IEngravingConfiguration> engravingConfiguration = { this };
    muse::Inject<muse::ui::IUiActionsRegister> uiActionsRegister = { this };

public:
    explicit NoteInputPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    Q_INVOKABLE QVariantList noteInputMethods() const;

    int defaultNoteInputMethod() const;
    bool addAccidentalDotsArticulationsToNextNoteEntered() const;
    bool useNoteInputCursorInInputByDuration() const;

    bool midiInputEnabled() const;
    bool startNoteInputAtSelectedNoteRestWhenPressingMidiKey() const;
    bool advanceToNextNoteOnKeyRelease() const;
    int delayBetweenNotesInRealTimeModeMilliseconds() const;

    bool playNotesWhenEditing() const;
    bool playPreviewNotesInInputByDuration() const;
    int notePlayDurationMilliseconds() const;
    bool playChordWhenEditing() const;
    bool playChordSymbolWhenEditing() const;
    bool playPreviewNotesWithScoreDynamics() const;
    bool playNotesOnMidiInput() const;

    bool dynamicsApplyToAllVoices() const;

    bool colorNotesOutsideOfUsablePitchRange() const;
    bool warnGuitarBends() const;

    bool autoUpdateFretboardDiagrams() const;

public slots:
    void setDefaultNoteInputMethod(int value);
    void setAddAccidentalDotsArticulationsToNextNoteEntered(bool value);
    void setUseNoteInputCursorInInputByDuration(bool value);

    void setMidiInputEnabled(bool value);
    void setStartNoteInputAtSelectedNoteRestWhenPressingMidiKey(bool value);
    void setAdvanceToNextNoteOnKeyRelease(bool value);
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delay);

    void setPlayNotesWhenEditing(bool value);
    void setPlayPreviewNotesInInputByDuration(bool value);
    void setNotePlayDurationMilliseconds(int duration);
    void setPlayChordWhenEditing(bool value);
    void setPlayChordSymbolWhenEditing(bool value);
    void setPlayPreviewNotesWithScoreDynamics(bool value);
    void setPlayNotesOnMidiInput(bool value);

    void setDynamicsApplyToAllVoices(bool value);

    void setColorNotesOutsideOfUsablePitchRange(bool value);
    void setWarnGuitarBends(bool value);

    void setAutoUpdateFretboardDiagrams(bool value);

signals:
    void defaultNoteInputMethodChanged(int value);
    void addAccidentalDotsArticulationsToNextNoteEnteredChanged(bool value);
    void useNoteInputCursorInInputByDurationChanged(bool value);

    void midiInputEnabledChanged(bool value);
    void startNoteInputAtSelectedNoteRestWhenPressingMidiKeyChanged(bool value);
    void advanceToNextNoteOnKeyReleaseChanged(bool value);
    void delayBetweenNotesInRealTimeModeMillisecondsChanged(int delay);

    void playNotesWhenEditingChanged(bool value);
    void playPreviewNotesInInputByDurationChanged(bool value);
    void notePlayDurationMillisecondsChanged(int duration);
    void playChordWhenEditingChanged(bool value);
    void playChordSymbolWhenEditingChanged(bool value);
    void playPreviewNotesWithScoreDynamicsChanged(bool value);
    void playNotesOnMidiInputChanged(bool value);

    void dynamicsApplyToAllVoicesChanged(bool value);

    void colorNotesOutsideOfUsablePitchRangeChanged(bool value);
    void warnGuitarBendsChanged(bool value);

    void autoUpdateFretboardDiagramsChanged(bool value);
};
}

#endif // MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H
