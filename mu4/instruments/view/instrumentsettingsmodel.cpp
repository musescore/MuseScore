//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "instrumentsettingsmodel.h"

#include "log.h"

using namespace mu::instruments;
using namespace mu::notation;

InstrumentSettingsModel::InstrumentSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentSettingsModel::load(const QVariant& instrument)
{
    QVariantMap map = instrument.toMap();

    m_partId = map["partId"].toString();
    m_instrumentId = map["instrumentId"].toString();

    parts()->instrumentChanged().onReceive(this, [this](const Instrument& instrument) {
        if (instrument.id == m_instrumentId) {
            emit dataChanged();
        }
    });

    parts()->partChanged().onReceive(this, [this](const Part* part) {
        for (const Instrument& instrument: parts()->instrumentList(part->id())) {
            if (instrument.id == m_instrumentId) {
                emit dataChanged();
                return;
            }
        }
    });

    emit dataChanged();
}

void InstrumentSettingsModel::replaceInstrument()
{
}

QString InstrumentSettingsModel::instrumentName() const
{
    Instrument instrument = this->instrument();
    return instrument.longNames.isEmpty() ? "" : instrument.longNames.first().name();
}

QString InstrumentSettingsModel::partName() const
{
    return part() ? part()->partName() : QString();
}

QString InstrumentSettingsModel::abbreviature() const
{
    Instrument instrument = this->instrument();
    return instrument.shortNames.isEmpty() ? "" : instrument.shortNames.first().name();
}

void InstrumentSettingsModel::setInstrumentName(const QString& name)
{
    if (parts()) {
        parts()->setInstrumentName(m_partId, m_instrumentId, name);
    }
}

void InstrumentSettingsModel::setPartName(const QString& name)
{
    if (parts()) {
        parts()->setPartName(m_partId, name);
    }
}

void InstrumentSettingsModel::setAbbreviature(const QString& abbreviature)
{
    if (parts()) {
        parts()->setInstrumentAbbreviature(m_partId, m_instrumentId, abbreviature);
    }
}

INotationParts* InstrumentSettingsModel::parts() const
{
    if (globalContext()->currentNotation()) {
        return globalContext()->currentNotation()->parts();
    }

    return nullptr;
}

Instrument InstrumentSettingsModel::instrument() const
{
    if (!parts()) {
        return Instrument();
    }

    InstrumentList instruments = parts()->instrumentList(m_partId);
    for (const Instrument& instrument: instruments) {
        if (instrument.id == m_instrumentId) {
            return instrument;
        }
    }

    return Instrument();
}

const Part* InstrumentSettingsModel::part() const
{
    if (!this->parts()) {
        return nullptr;
    }

    PartList parts = this->parts()->partList();
    for (const Part* part: parts) {
        if (part->id() == m_partId) {
            return part;
        }
    }

    return nullptr;
}
