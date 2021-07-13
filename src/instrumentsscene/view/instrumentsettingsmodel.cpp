/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "instrumentsettingsmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

InstrumentSettingsModel::InstrumentSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentSettingsModel::load(const QVariant& instrument)
{
    QVariantMap map = instrument.toMap();

    m_partId = map["partId"].toString();
    m_partName = map["partName"].toString();
    m_instrumentId = map["instrumentId"].toString();
    m_instrumentName = map["instrumentName"].toString();
    m_instrumentAbbreviature = map["abbreviature"].toString();

    emit dataChanged();
}

void InstrumentSettingsModel::replaceInstrument()
{
    if (!parts()) {
        return;
    }

    RetVal<Instrument> selectedInstrument = selectInstrumentsScenario()->selectInstrument(m_instrumentId.toStdString());
    if (!selectedInstrument.ret) {
        LOGE() << selectedInstrument.ret.toString();
        return;
    }

    Instrument newInstrument = selectedInstrument.val;
    parts()->replaceInstrument(m_instrumentId, m_partId, newInstrument);

    m_instrumentId = newInstrument.id;
    m_instrumentName = newInstrument.name;
    m_instrumentAbbreviature = newInstrument.abbreviature();

    emit dataChanged();
}

QString InstrumentSettingsModel::instrumentName() const
{
    return m_instrumentName;
}

QString InstrumentSettingsModel::partName() const
{
    return m_partName;
}

QString InstrumentSettingsModel::abbreviature() const
{
    return m_instrumentAbbreviature;
}

void InstrumentSettingsModel::setInstrumentName(const QString& name)
{
    if (m_instrumentName == name || !parts()) {
        return;
    }

    m_instrumentName = name;
    parts()->setInstrumentName(m_instrumentId, m_partId, name);
}

void InstrumentSettingsModel::setPartName(const QString& name)
{
    if (m_partName == name || !parts()) {
        return;
    }

    m_partName = name;
    parts()->setPartName(m_partId, name);
}

void InstrumentSettingsModel::setAbbreviature(const QString& abbreviature)
{
    if (m_instrumentAbbreviature == abbreviature || !parts()) {
        return;
    }

    m_instrumentAbbreviature = abbreviature;
    parts()->setInstrumentAbbreviature(m_instrumentId, m_partId, abbreviature);
}

INotationPartsPtr InstrumentSettingsModel::parts() const
{
    if (context()->currentMasterNotation()) {
        return context()->currentMasterNotation()->parts();
    }

    return nullptr;
}
