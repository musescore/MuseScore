/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "part.h"

#include <QJSEngine>

#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/score.h"

// api
#include "apistructs.h"
#include "elements.h"
#include "instrument.h"

using namespace mu::engraving::apiv1;

MixerChannel::MixerChannel(
    const mu::engraving::InstrumentTrackId &instrumentTrackId,
    mu::engraving::Part *part, QObject *parent)
    : QObject(parent), muse::Contextable(part->score()->iocContext()),
      mixerApi(part->score()->iocContext()),
      m_instrumentTrackId(instrumentTrackId), m_part(part) {}

InstrumentListProperty::InstrumentListProperty(Part *p)
    : QQmlListProperty<Instrument>(p, p, &count, &at) {}

//---------------------------------------------------------
//   InstrumentListProperty::count
//---------------------------------------------------------

qsizetype InstrumentListProperty::count(QQmlListProperty<Instrument> *l) {
  return static_cast<qsizetype>(
      static_cast<Part *>(l->data)->part()->instruments().size());
}

//---------------------------------------------------------
//   InstrumentListProperty::at
//---------------------------------------------------------

Instrument *InstrumentListProperty::at(QQmlListProperty<Instrument> *l,
                                       qsizetype i) {
  Part *part = static_cast<Part *>(l->data);
  const mu::engraving::InstrumentList &il = part->part()->instruments();

  if (i < 0 || i >= int(il.size())) {
    return nullptr;
  }

  mu::engraving::Instrument *instr = std::next(il.begin(), i)->second;

  return customWrap<Instrument>(instr, part->part());
}

//---------------------------------------------------------
//   Part::instruments
//---------------------------------------------------------

InstrumentListProperty Part::instruments() {
  return InstrumentListProperty(this);
}

MixerChannel *Part::mixerChannel() {
  if (m_mixerChannel) {
    return m_mixerChannel;
  }

  const mu::engraving::InstrumentTrackIdList trackIds =
      part()->instrumentTrackIdList();
  if (trackIds.empty()) {
    return nullptr;
  }

  m_mixerChannel = new MixerChannel(trackIds.front(), part(), this);
  return m_mixerChannel;
}

float MixerChannel::volume() const {
  return mixerApi()->volume(m_instrumentTrackId);
}

void MixerChannel::setVolume(float volume) {
  mixerApi()->setVolume(m_instrumentTrackId, volume);
}

float MixerChannel::balance() const {
  return mixerApi()->balance(m_instrumentTrackId);
}

void MixerChannel::setBalance(float balance) {
  mixerApi()->setBalance(m_instrumentTrackId, balance);
}

bool MixerChannel::muted() const {
  return mixerApi()->muted(m_instrumentTrackId);
}

void MixerChannel::setMuted(bool muted) {
  mixerApi()->setMuted(m_instrumentTrackId, muted);
}

bool MixerChannel::solo() const {
  return mixerApi()->solo(m_instrumentTrackId);
}

void MixerChannel::setSolo(bool solo) {
  mixerApi()->setSolo(m_instrumentTrackId, solo);
}

int MixerChannel::midiBank() const {
  mu::engraving::Instrument *instrument =
      const_cast<mu::engraving::Instrument *>(
          m_part->instrumentById(m_instrumentTrackId.instrumentId));
  if (!instrument || instrument->channel().empty()) {
    return 0;
  }

  Channel channel(instrument->channel(0), m_part);
  return channel.midiBank();
}

void MixerChannel::setMidiBank(int bank) {
  mu::engraving::Instrument *instrument =
      const_cast<mu::engraving::Instrument *>(
          m_part->instrumentById(m_instrumentTrackId.instrumentId));
  if (!instrument || instrument->channel().empty()) {
    return;
  }

  Channel channel(instrument->channel(0), m_part);
  channel.setMidiBank(bank);
}

int MixerChannel::midiProgram() const {
  mu::engraving::Instrument *instrument =
      const_cast<mu::engraving::Instrument *>(
          m_part->instrumentById(m_instrumentTrackId.instrumentId));
  if (!instrument || instrument->channel().empty()) {
    return 0;
  }

  Channel channel(instrument->channel(0), m_part);
  return channel.midiProgram();
}

void MixerChannel::setMidiProgram(int program) {
  mu::engraving::Instrument *instrument =
      const_cast<mu::engraving::Instrument *>(
          m_part->instrumentById(m_instrumentTrackId.instrumentId));
  if (!instrument || instrument->channel().empty()) {
    return;
  }

  Channel channel(instrument->channel(0), m_part);
  channel.setMidiProgram(program);
}

void MixerChannel::availableSounds(QJSValue callback) {
  mixerApi()
      ->availableSounds(m_instrumentTrackId)
      .onResolve(this,
                 [this, callback](const QVariantList &sounds) {
                   QJSEngine *engine = qjsEngine(this);
                   callback.call(
                       {engine ? engine->toScriptValue(sounds) : QJSValue(),
                        QJSValue()});
                 })
      .onReject(this, [callback](int, const std::string &error) {
        callback.call({QJSValue(), QJSValue(QString::fromStdString(error))});
      });
}

void MixerChannel::setSound(const QString &soundId, QJSValue callback) {
  mixerApi()
      ->setSound(m_instrumentTrackId, soundId)
      .onResolve(this,
                 [callback](bool success) {
                   callback.call({QJSValue(success), QJSValue()});
                 })
      .onReject(this, [callback](int, const std::string &error) {
        callback.call(
            {QJSValue(false), QJSValue(QString::fromStdString(error))});
      });
}

//---------------------------------------------------------
//   Part::instrumentAtTick
//---------------------------------------------------------

Instrument *Part::instrumentAtTick(int tick) {
  return customWrap<Instrument>(
      part()->instrument(mu::engraving::Fraction::fromTicks(tick)), part());
}

Instrument *Part::instrumentAtTick(Fraction *tick) {
  return customWrap<Instrument>(part()->instrument(tick->fraction()), part());
}

QQmlListProperty<Staff> Part::staves() {
  return wrapContainerProperty<Staff>(this, part()->staves());
}

QString Part::longNameAtTick(Fraction *tick) {
  return part()->longName(tick->fraction());
}

QString Part::shortNameAtTick(Fraction *tick) {
  return part()->shortName(tick->fraction());
}

QString Part::instrumentNameAtTick(Fraction *tick) {
  return part()->instrumentName(tick->fraction());
}

QString Part::instrumentIdAtTick(Fraction *tick) {
  return part()->instrumentId(tick->fraction());
}

EngravingItem *Part::currentHarpDiagramAtTick(Fraction *tick) {
  return wrap(part()->currentHarpDiagram(tick->fraction()));
}

EngravingItem *Part::nextHarpDiagramFromTick(Fraction *tick) {
  return wrap(part()->nextHarpDiagram(tick->fraction()));
}

EngravingItem *Part::prevHarpDiagramFromTick(Fraction *tick) {
  return wrap(part()->prevHarpDiagram(tick->fraction()));
}

Fraction *Part::tickOfCurrentHarpDiagram(Fraction *tick) {
  return wrap(part()->currentHarpDiagramTick(tick->fraction()));
}
