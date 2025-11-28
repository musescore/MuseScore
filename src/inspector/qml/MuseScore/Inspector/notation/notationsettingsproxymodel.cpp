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
#include "notationsettingsproxymodel.h"

#include "translation.h"

using namespace mu::inspector;

NotationSettingsProxyModel::NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository,
                                                       const ElementKeySet& elementKeySet)
    : AbstractInspectorProxyModel(parent, repository)
{
    connect(this, &AbstractInspectorProxyModel::modelsChanged, [this]() {
        auto models = modelList();

        if (models.size() == 1) {
            setTitle(models.first()->title());
        } else {
            setTitle(muse::qtrc("inspector", "Notation"));
        }
    });

    setSectionType(InspectorSectionType::SECTION_NOTATION);
    updateModels(elementKeySet);
}
