/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "restsettingsproxymodel.h"
#include "beams/restbeamsettingsmodel.h"
#include "restsettingsmodel.h"

#include "dom/rest.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

RestSettingsProxyModel::RestSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_REST);
    setTitle(muse::qtrc("inspector", "Rest"));
    setIcon(muse::ui::IconCode::Code::REST_8TH);

    QList<AbstractInspectorModel*> models {
        new RestBeamSettingsModel(this, repository),
        new RestSettingsModel(this, repository)
    };

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);
}
