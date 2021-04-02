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
#ifndef MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H
#define MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "playback/iplaybackconfiguration.h"

namespace mu::appshell {
class NoteInputPreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, playback::IPlaybackConfiguration, playbackConfiguration)

    Q_PROPERTY(
        bool advanceToNextNoteOnKeyRelease READ advanceToNextNoteOnKeyRelease WRITE setAdvanceToNextNoteOnKeyRelease NOTIFY advanceToNextNoteOnKeyReleaseChanged)
    Q_PROPERTY(
        bool colorNotesOusideOfUsablePitchRange READ colorNotesOusideOfUsablePitchRange WRITE setColorNotesOusideOfUsablePitchRange NOTIFY colorNotesOusideOfUsablePitchRangeChanged)
    Q_PROPERTY(
        int delayBetweenNotesInRealTimeModeMilliseconds READ delayBetweenNotesInRealTimeModeMilliseconds WRITE setDelayBetweenNotesInRealTimeModeMilliseconds NOTIFY delayBetweenNotesInRealTimeModeMillisecondsChanged)

    Q_PROPERTY(bool playNotesWhenEditing READ playNotesWhenEditing WRITE setPlayNotesWhenEditing NOTIFY playNotesWhenEditingChanged)
    Q_PROPERTY(
        int notePlayDurationMilliseconds READ notePlayDurationMilliseconds WRITE setNotePlayDurationMilliseconds NOTIFY notePlayDurationMillisecondsChanged)
    Q_PROPERTY(bool playChordWhenEditing READ playChordWhenEditing WRITE setPlayChordWhenEditing NOTIFY playChordWhenEditingChanged)
    Q_PROPERTY(
        bool playChordSymbolWhenEditing READ playChordSymbolWhenEditing WRITE setPlayChordSymbolWhenEditing NOTIFY playChordSymbolWhenEditingChanged)

public:
    explicit NoteInputPreferencesModel(QObject* parent = nullptr);

    bool advanceToNextNoteOnKeyRelease() const;
    bool colorNotesOusideOfUsablePitchRange() const;
    int delayBetweenNotesInRealTimeModeMilliseconds() const;

    bool playNotesWhenEditing() const;
    int notePlayDurationMilliseconds() const;
    bool playChordWhenEditing() const;
    bool playChordSymbolWhenEditing() const;

public slots:
    void setAdvanceToNextNoteOnKeyRelease(bool value);
    void setColorNotesOusideOfUsablePitchRange(bool value);
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delay);
    void setPlayNotesWhenEditing(bool value);
    void setNotePlayDurationMilliseconds(int duration);
    void setPlayChordWhenEditing(bool value);
    void setPlayChordSymbolWhenEditing(bool value);

signals:
    void advanceToNextNoteOnKeyReleaseChanged(bool value);
    void colorNotesOusideOfUsablePitchRangeChanged(bool value);
    void delayBetweenNotesInRealTimeModeMillisecondsChanged(int delay);
    void playNotesWhenEditingChanged(bool value);
    void notePlayDurationMillisecondsChanged(int duration);
    void playChordWhenEditingChanged(bool value);
    void playChordSymbolWhenEditingChanged(bool value);
};
}

#endif // MU_APPSHELL_NOTEINPUTPREFERENCESMODEL_H
