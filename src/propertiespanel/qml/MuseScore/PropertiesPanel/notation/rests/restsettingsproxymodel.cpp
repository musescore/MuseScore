/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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

#include "translation.h"

using namespace mu::propertiespanel;

RestSettingsProxyModel::RestSettingsProxyModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                               IElementRepositoryService* repository)
    : PropertiesPanelAbstractProxyModel(parent, iocCtx, repository)
{
    setModelType(PropertiesPanelModelType::TYPE_REST);
    setTitle(muse::qtrc("propertiespanel", "Rest"));
    setIcon(muse::ui::IconCode::Code::REST_8TH);

    QList<PropertiesPanelAbstractModel*> models {
        new RestBeamSettingsModel(this, iocCtx, repository),
        new RestSettingsModel(this, iocCtx, repository)
    };

    for (PropertiesPanelAbstractModel* model : models) {
        model->init();
    }

    setModels(models);
}
