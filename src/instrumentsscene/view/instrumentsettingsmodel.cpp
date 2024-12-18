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
#include "instrumentsettingsmodel.h"

using namespace muse;
using namespace mu::instrumentsscene;
using namespace mu::notation;

InstrumentSettingsModel::InstrumentSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentSettingsModel::load(const QVariant& instrument)
{
    if (!notationParts()) {
        return;
    }

    QVariantMap map = instrument.toMap();
    m_instrumentKey.partId = ID(map["partId"]);
    m_instrumentKey.instrumentId = map["instrumentId"].toString();
    m_instrumentKey.tick = Part::MAIN_INSTRUMENT_TICK;

    const Part* part = notationParts()->part(m_instrumentKey.partId);
    if (!part) {
        return;
    }

    m_instrumentName = part->instrument()->nameAsPlainText();
    m_instrumentAbbreviature = part->instrument()->abbreviatureAsPlainText();

    context()->currentNotationChanged().onNotify(this, [this]() {
        emit isMainScoreChanged();
    });

    emit dataChanged();
}

QString InstrumentSettingsModel::instrumentName() const
{
    return m_instrumentName;
}

QString InstrumentSettingsModel::abbreviature() const
{
    return m_instrumentAbbreviature;
}

bool InstrumentSettingsModel::isMainScore() const
{
    return currentNotation() == currentMasterNotation();
}

void InstrumentSettingsModel::setInstrumentName(const QString& name)
{
    if (m_instrumentName == name || !notationParts()) {
        return;
    }

    m_instrumentName = name;
    notationParts()->setInstrumentName(m_instrumentKey, name);
}

void InstrumentSettingsModel::setAbbreviature(const QString& abbreviature)
{
    if (m_instrumentAbbreviature == abbreviature || !notationParts()) {
        return;
    }

    m_instrumentAbbreviature = abbreviature;
    notationParts()->setInstrumentAbbreviature(m_instrumentKey, abbreviature);
}

INotationPtr InstrumentSettingsModel::currentNotation() const
{
    return context()->currentNotation();
}

INotationPtr InstrumentSettingsModel::currentMasterNotation() const
{
    IMasterNotationPtr master = context()->currentMasterNotation();
    return master ? master->notation() : nullptr;
}

INotationPartsPtr InstrumentSettingsModel::notationParts() const
{
    INotationPtr notation = currentNotation();
    return notation ? notation->parts() : nullptr;
}
