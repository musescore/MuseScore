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
using namespace mu::framework;

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

void InstrumentSettingsModel::replaceInstrument()
{
    if (!masterNotationParts()) {
        return;
    }

    RetVal<Instrument> selectedInstrument = selectInstrumentsScenario()->selectInstrument(m_instrumentKey);
    if (!selectedInstrument.ret) {
        LOGE() << selectedInstrument.ret.toString();
        return;
    }

    const Instrument& newInstrument = selectedInstrument.val;
    masterNotationParts()->replaceInstrument(m_instrumentKey, newInstrument);

    m_instrumentKey.instrumentId = newInstrument.id();
    m_instrumentName = newInstrument.nameAsPlainText();
    m_instrumentAbbreviature = newInstrument.abbreviatureAsPlainText();

    emit dataChanged();
}

void InstrumentSettingsModel::resetAllFormatting()
{
    if (!masterNotationParts() || !notationParts()) {
        return;
    }

    std::string title = mu::trc("instruments", "Are you sure you want to reset all formatting?");
    std::string body = mu::trc("instruments", "This action can not be undone");

    IInteractive::Button button = interactive()->question(title, body, {
        IInteractive::Button::No,
        IInteractive::Button::Yes
    }).standardButton();

    if (button == IInteractive::Button::No) {
        return;
    }

    const Part* masterPart = masterNotationParts()->part(m_instrumentKey.partId);
    notationParts()->replacePart(m_instrumentKey.partId, masterPart->clone());
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

INotationPartsPtr InstrumentSettingsModel::masterNotationParts() const
{
    INotationPtr notation = currentMasterNotation();
    return notation ? notation->parts() : nullptr;
}
