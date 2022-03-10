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

#include "instrumentnamesettingsmodel.h"

#include "engraving/libmscore/instrumentname.h"

using namespace mu::inspector;
using namespace mu::actions;

InstrumentNameSettingsModel::InstrumentNameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Instrument names"));
    setModelType(InspectorModelType::TYPE_INSTRUMENT_NAME);
}

void InstrumentNameSettingsModel::openStyleSettings()
{
    const Ms::InstrumentName* instrumentName = selectedInstrumentName();
    IF_ASSERT_FAILED(instrumentName) {
        return;
    }

    QString subPageCode = "instrument-name-long";

    if (instrumentName->instrumentNameType() == Ms::InstrumentNameType::SHORT) {
        subPageCode = "instrument-name-short";
    }

    ActionData args = ActionData::make_arg2<QString, QString>("text-styles", subPageCode);
    dispatcher()->dispatch("edit-style", args);
}

void InstrumentNameSettingsModel::openStaffAndPartProperties()
{
    dispatcher()->dispatch("staff-properties");
}

const Ms::InstrumentName* InstrumentNameSettingsModel::selectedInstrumentName() const
{
    for (Ms::EngravingItem* element : selection()->elements()) {
        if (element->isInstrumentName()) {
            return Ms::toInstrumentName(element);
        }
    }

    return nullptr;
}
