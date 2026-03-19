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

#include "instrument.h"

#include "engraving/compat/midi/midipatch.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/part.h"
#include "engraving/dom/masterscore.h"

#include "engraving/editing/editpart.h"

using namespace mu::engraving::apiv1;

//---------------------------------------------------------
//   Channel::activeChannel
//---------------------------------------------------------

mu::engraving::InstrChannel* Channel::activeChannel()
{
    mu::engraving::Score* score = m_part->score();
    mu::engraving::MasterScore* masterScore = score->masterScore();

    if (masterScore->playbackScore() == score) {
        return masterScore->playbackChannel(m_channel);
    }
    return m_channel;
}

//---------------------------------------------------------
//   Channel::setMidiBankAndProgram
//---------------------------------------------------------

void Channel::setMidiBankAndProgram(int bank, int program, bool setUserBankController)
{
    mu::engraving::InstrChannel* ch = activeChannel();

    MidiPatch patch;
    // other values are unused in ChangePatch command
    patch.synti = ch->synti();
    patch.bank = bank;
    patch.prog = program;

    mu::engraving::Score* score = m_part->score();
    score->undo(new ChangePatch(score, ch, patch));

    if (setUserBankController) {
        score->undo(new SetUserBankController(ch, true));
    }
}

//---------------------------------------------------------
//   Channel::setMidiProgram
//---------------------------------------------------------

void Channel::setMidiProgram(int prog)
{
    prog = qBound(0, prog, 127);
    setMidiBankAndProgram(activeChannel()->bank(), prog, false);
}

//---------------------------------------------------------
//   Channel::setMidiBank
//---------------------------------------------------------

void Channel::setMidiBank(int bank)
{
    bank = qBound(0, bank, 255);
    setMidiBankAndProgram(bank, activeChannel()->program(), true);
}

//---------------------------------------------------------
//   StringData::stringList
//---------------------------------------------------------

QVariantList StringData::stringList() const
{
    QVariantList pluginStringsList;
    for (instrString str : m_data.stringList()) {
        QVariantMap pluginStringData;
        pluginStringData["pitch"] = str.pitch;
        pluginStringData["open"] = str.open;
        pluginStringsList.append(pluginStringData);
    }
    return pluginStringsList;
}

//---------------------------------------------------------
//   Drumset setters
//   These allow modifying a cloned drumset before passing
//   it to Score::replaceDrumset(). The Drumset obtained
//   from an Instrument is read-only; Instrument::cloneDrumset()
//   returns a writable copy that can be modified with these
//   setters and then applied to the score.
//---------------------------------------------------------

void Drumset::setName(int pitch, const QString& name)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).name = name;
}

void Drumset::setNoteHead(int pitch, int noteHead)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).notehead = mu::engraving::NoteHeadGroup(noteHead);
}

void Drumset::setLine(int pitch, int line)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).line = line;
}

void Drumset::setVoice(int pitch, int voice)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).voice = voice;
}

void Drumset::setStemDirection(int pitch, int stemDirection)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).stemDirection = mu::engraving::DirectionV(stemDirection);
}

void Drumset::setShortcut(int pitch, const QString& shortcut)
{
    IF_ASSERT_FAILED(pitch >= 0 && pitch < DRUM_INSTRUMENTS) {
        return;
    }
    m_drumset->drum(pitch).shortcut = shortcut;
}

//---------------------------------------------------------
//   Drumset::variants
//---------------------------------------------------------

QVariantList Drumset::variants(int pitch)
{
    QVariantList drumInstrumentVariantList;
    for (const DrumInstrumentVariant& div : m_drumset->variants(pitch)) {
        QVariantMap pluginDivData;
        pluginDivData["pitch"] = div.pitch;
        pluginDivData["tremolo"] = int(div.tremolo);
        pluginDivData["articulationName"] = div.articulationName.toQString();
        drumInstrumentVariantList.append(pluginDivData);
    }
    return drumInstrumentVariantList;
}

//---------------------------------------------------------
//   ChannelListProperty
//---------------------------------------------------------

ChannelListProperty::ChannelListProperty(Instrument* i)
    : QQmlListProperty<Channel>(i, i, nullptr, &count, &at, nullptr) {}

//---------------------------------------------------------
//   ChannelListProperty::count
//---------------------------------------------------------

qsizetype ChannelListProperty::count(QQmlListProperty<Channel>* l)
{
    return static_cast<Instrument*>(l->data)->instrument()->channel().size();
}

//---------------------------------------------------------
//   ChannelListProperty::at
//---------------------------------------------------------

Channel* ChannelListProperty::at(QQmlListProperty<Channel>* l, qsizetype i)
{
    Instrument* instr = static_cast<Instrument*>(l->data);

    if (i < 0 || size_t(i) >= instr->instrument()->channel().size()) {
        return nullptr;
    }

    mu::engraving::InstrChannel* ch = instr->instrument()->channel(i);

    return customWrap<Channel>(ch, instr->part());
}

//---------------------------------------------------------
//   Instrument::longName
//---------------------------------------------------------

QString Instrument::longName() const
{
    return instrument()->longName().toQString();
}

//---------------------------------------------------------
//   Instrument::shortName
//---------------------------------------------------------

QString Instrument::shortName() const
{
    return instrument()->shortName().toQString();
}

//---------------------------------------------------------
//   Instrument::channels
//---------------------------------------------------------

Drumset* Instrument::cloneDrumset()
{
    const mu::engraving::Drumset* ds = instrument()->drumset();
    if (!ds) {
        return nullptr;
    }

    return new Drumset(new mu::engraving::Drumset(*ds), true, this);
}

ChannelListProperty Instrument::channels()
{
    return ChannelListProperty(this);
}
