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

#include "instrumentnamesettingsmodel.h"

#include "engraving/dom/instrumentname.h"

#include "notation/inotationselection.h" // IWYU pragma: keep

using namespace mu::propertiespanel;
using namespace muse::actions;

InstrumentNameSettingsModel::InstrumentNameSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                         IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setTitle(muse::qtrc("propertiespanel", "Instrument names"));
    setModelType(PropertiesPanelModelType::TYPE_INSTRUMENT_NAME);
}

void InstrumentNameSettingsModel::openStyleSettings()
{
    const mu::engraving::InstrumentName* instrumentName = selectedInstrumentName();
    IF_ASSERT_FAILED(instrumentName) {
        return;
    }

    std::string subPageCode = "instrument-name-long";

    if (instrumentName->instrumentNameType() == mu::engraving::InstrumentNameType::SHORT) {
        subPageCode = "instrument-name-short";
    }

    ActionData args = ActionData::make_arg2<std::string, std::string>("text-styles", subPageCode);
    dispatcher()->dispatch("edit-style", args);
}

void InstrumentNameSettingsModel::openStaffAndPartProperties()
{
    dispatcher()->dispatch("staff-properties");
}

const mu::engraving::InstrumentName* InstrumentNameSettingsModel::selectedInstrumentName() const
{
    for (mu::engraving::EngravingItem* element : selection()->elements()) {
        if (element->isInstrumentName()) {
            return mu::engraving::toInstrumentName(element);
        }
    }

    return nullptr;
}
