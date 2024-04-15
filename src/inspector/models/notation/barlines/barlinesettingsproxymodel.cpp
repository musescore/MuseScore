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
#include "barlinesettingsproxymodel.h"

#include "translation.h"

#include "barlinesettingsmodel.h"
#include "../staffs/staffsettingsmodel.h"

using namespace mu::inspector;

BarlineSettingsProxyModel::BarlineSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BARLINE);
    setTitle(muse::qtrc("inspector", "Barline"));
    setIcon(muse::ui::IconCode::Code::SECTION_BREAK);

    QList<AbstractInspectorModel*> models {
        new BarlineSettingsModel(this, repository),
        new StaffSettingsModel(this, repository)
    };

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);
}
