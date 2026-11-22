/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "midi/imidiinport.h"
#include "notation/inotationconfiguration.h"

namespace mu::playback {
//! The split point of the MIDI recording on two staves: chosen from a list, or learnt from the MIDI keyboard
class MidiRecordingSplitPointModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(int splitPitch READ splitPitch WRITE setSplitPitch NOTIFY splitPitchChanged)
    Q_PROPERTY(QString splitPitchName READ splitPitchName NOTIFY splitPitchChanged)
    Q_PROPERTY(QVariantList pitches READ pitches CONSTANT)
    Q_PROPERTY(bool isLearning READ isLearning NOTIFY isLearningChanged)

    QML_ELEMENT

    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<muse::midi::IMidiInPort> midiInPort;

public:
    explicit MidiRecordingSplitPointModel(QObject* parent = nullptr);
    ~MidiRecordingSplitPointModel() override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void toggleLearning();
    Q_INVOKABLE void apply();
    Q_INVOKABLE void cancel();

    int splitPitch() const;
    void setSplitPitch(int pitch);
    QString splitPitchName() const;

    QVariantList pitches() const;
    bool isLearning() const;

    //! e.g. 60 -> "C4"
    static QString pitchName(int pitch);

signals:
    void splitPitchChanged();
    void isLearningChanged();

private:
    void startLearning();
    void stopLearning();

    int m_splitPitch = 60;
    bool m_isLearning = false;
    bool m_midiInputWasEnabled = false;
};
}
