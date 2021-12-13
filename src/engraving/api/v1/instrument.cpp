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
#include "engraving/dom/part.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/undo.h"

using namespace mu::engraving::apiv1;

//---------------------------------------------------------
//   Channel::activeChannel
//---------------------------------------------------------

mu::engraving::InstrChannel* Channel::activeChannel()
{
    mu::engraving::Score* score = _part->score();
    mu::engraving::MasterScore* masterScore = score->masterScore();

    if (masterScore->playbackScore() == score) {
        return masterScore->playbackChannel(_channel);
    }
    return _channel;
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

    mu::engraving::Score* score = _part->score();
    score->undo(new ChangePatch(score, ch, &patch));

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
    for (instrString str : _data.stringList()) {
        QVariantMap pluginStringData;
        pluginStringData["pitch"] = str.pitch;
        pluginStringData["open"] = str.open;
        pluginStringsList.append(pluginStringData);
    }
    return pluginStringsList;
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
    const std::list<mu::engraving::StaffName>& names = instrument()->longNames();
    return names.empty() ? u"" : names.front().name();
}

//---------------------------------------------------------
//   Instrument::shortName
//---------------------------------------------------------

QString Instrument::shortName() const
{
    const std::list<mu::engraving::StaffName>& names = instrument()->shortNames();
    return names.empty() ? u"" : names.front().name();
}

//---------------------------------------------------------
//   Instrument::channels
//---------------------------------------------------------

ChannelListProperty Instrument::channels()
{
    return ChannelListProperty(this);
}
