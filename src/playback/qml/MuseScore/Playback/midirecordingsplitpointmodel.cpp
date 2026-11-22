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
#include "midirecordingsplitpointmodel.h"

#include <algorithm>

#include <QVariantMap>

#include "translation.h"

using namespace mu::playback;

//! The range of a piano keyboard
static constexpr int LOWEST_PITCH = 21;   // A0
static constexpr int HIGHEST_PITCH = 108; // C8
static constexpr int MIDDLE_C = 60;

MidiRecordingSplitPointModel::MidiRecordingSplitPointModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

MidiRecordingSplitPointModel::~MidiRecordingSplitPointModel()
{
    stopLearning();
}

void MidiRecordingSplitPointModel::load()
{
    m_splitPitch = std::clamp(notationConfiguration()->midiRecordingSplitPitch(), LOWEST_PITCH, HIGHEST_PITCH);
    emit splitPitchChanged();
}

int MidiRecordingSplitPointModel::splitPitch() const
{
    return m_splitPitch;
}

void MidiRecordingSplitPointModel::setSplitPitch(int pitch)
{
    pitch = std::clamp(pitch, LOWEST_PITCH, HIGHEST_PITCH);
    if (pitch == m_splitPitch) {
        return;
    }

    m_splitPitch = pitch;
    emit splitPitchChanged();
}

QString MidiRecordingSplitPointModel::splitPitchName() const
{
    return pitchName(m_splitPitch);
}

QString MidiRecordingSplitPointModel::pitchName(int pitch)
{
    static const char* NAMES[] = { "C", "C♯", "D", "E♭", "E", "F", "F♯", "G", "A♭", "A", "B♭", "B" };

    const int octave = pitch / 12 - 1;
    return QString("%1%2").arg(QString::fromUtf8(NAMES[pitch % 12])).arg(octave);
}

QVariantList MidiRecordingSplitPointModel::pitches() const
{
    QVariantList result;

    // From the highest, like a vertical keyboard
    for (int pitch = HIGHEST_PITCH; pitch >= LOWEST_PITCH; --pitch) {
        QString text = pitchName(pitch);
        if (pitch == MIDDLE_C) {
            text = muse::qtrc("playback", "%1 (middle C)").arg(text);
        }

        QVariantMap item;
        item["text"] = text;
        item["value"] = pitch;
        result << item;
    }

    return result;
}

bool MidiRecordingSplitPointModel::isLearning() const
{
    return m_isLearning;
}

void MidiRecordingSplitPointModel::toggleLearning()
{
    if (m_isLearning) {
        stopLearning();
    } else {
        startLearning();
    }
}

void MidiRecordingSplitPointModel::startLearning()
{
    if (m_isLearning || !midiInPort()) {
        return;
    }

    //! NOTE While learning, the key must not be used for note input: the MIDI input of the score is paused
    m_midiInputWasEnabled = notationConfiguration()->isMidiInputEnabled();
    if (m_midiInputWasEnabled) {
        notationConfiguration()->setIsMidiInputEnabled(false);
    }

    midiInPort()->eventReceived().onReceive(this, [this](const muse::midi::tick_t, const muse::midi::Event& event) {
        if (!m_isLearning) {
            return;
        }

        std::vector<muse::midi::Event> events;
        if (event.isChannelVoice20()) {
            events = event.toMIDI10();
        } else {
            events.push_back(event);
        }

        for (const muse::midi::Event& e : events) {
            if (e.opcode() == muse::midi::Event::Opcode::NoteOn && e.velocity7() > 0) {
                setSplitPitch(e.note());
                stopLearning();
                return;
            }
        }
    });

    m_isLearning = true;
    emit isLearningChanged();
}

void MidiRecordingSplitPointModel::stopLearning()
{
    if (!m_isLearning) {
        return;
    }

    m_isLearning = false;

    if (midiInPort()) {
        midiInPort()->eventReceived().disconnect(this);
    }

    if (m_midiInputWasEnabled) {
        notationConfiguration()->setIsMidiInputEnabled(true);
        m_midiInputWasEnabled = false;
    }

    emit isLearningChanged();
}

void MidiRecordingSplitPointModel::apply()
{
    stopLearning();
    notationConfiguration()->setMidiRecordingSplitPitch(m_splitPitch);
}

void MidiRecordingSplitPointModel::cancel()
{
    stopLearning();
}
