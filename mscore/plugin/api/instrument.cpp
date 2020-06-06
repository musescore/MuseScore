//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
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

#include "instrument.h"

#include "audio/midi/midipatch.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Channel::activeChannel
//---------------------------------------------------------

Ms::Channel* Channel::activeChannel()
      {
      Ms::Score* score = _part->score();
      Ms::MasterScore* masterScore = score->masterScore();

      if (masterScore->playbackScore() == score)
            return masterScore->playbackChannel(_channel);
      return _channel;
      }

//---------------------------------------------------------
//   Channel::setMidiBankAndProgram
//---------------------------------------------------------

void Channel::setMidiBankAndProgram(int bank, int program, bool setUserBankController)
      {
      Ms::Channel* ch = activeChannel();

      MidiPatch patch;
      // other values are unused in ChangePatch command
      patch.synti = ch->synti();
      patch.bank = bank;
      patch.prog = program;

      Ms::Score* score = _part->score();
      score->undo(new ChangePatch(score, ch, &patch));

      if (setUserBankController)
            score->undo(new SetUserBankController(ch, true));
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
   : QQmlListProperty<Channel>(i, i, &count, &at) {}

//---------------------------------------------------------
//   ChannelListProperty::count
//---------------------------------------------------------

int ChannelListProperty::count(QQmlListProperty<Channel>* l)
      {
      return static_cast<int>(static_cast<Instrument*>(l->data)->instrument()->channel().size());
      }

//---------------------------------------------------------
//   ChannelListProperty::at
//---------------------------------------------------------

Channel* ChannelListProperty::at(QQmlListProperty<Channel>* l, int i)
      {
      Instrument* instr = static_cast<Instrument*>(l->data);

      if (i < 0 || i >= instr->instrument()->channel().size())
            return nullptr;

      Ms::Channel* ch = instr->instrument()->channel(i);

      return customWrap<Channel>(ch, instr->part());
      }

//---------------------------------------------------------
//   Instrument::longName
//---------------------------------------------------------

QString Instrument::longName() const
      {
      const QList<Ms::StaffName>& names = instrument()->longNames();
      return names.empty() ? "" : names[0].name();
      }

//---------------------------------------------------------
//   Instrument::shortName
//---------------------------------------------------------

QString Instrument::shortName() const
      {
      const QList<Ms::StaffName>& names = instrument()->shortNames();
      return names.empty() ? "" : names[0].name();
      }

//---------------------------------------------------------
//   Instrument::channels
//---------------------------------------------------------

ChannelListProperty Instrument::channels()
      {
      return ChannelListProperty(this);
      }

} // namespace PluginAPI
} // namespace Ms
