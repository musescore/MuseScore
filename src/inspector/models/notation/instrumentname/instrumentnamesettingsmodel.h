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
#ifndef MU_INSPECTOR_INSTRUMENTNAMESETTINGSMODEL_H
#define MU_INSPECTOR_INSTRUMENTNAMESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace mu::inspector {
class InstrumentNameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    INJECT(muse::actions::IActionsDispatcher, dispatcher)

public:
    explicit InstrumentNameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void openStyleSettings();
    Q_INVOKABLE void openStaffAndPartProperties();

private:
    const mu::engraving::InstrumentName* selectedInstrumentName() const;

    void createProperties() override {}
    void loadProperties() override {}
    void resetProperties() override {}
};
}

#endif // MU_INSPECTOR_INSTRUMENTNAMESETTINGSMODEL_H
