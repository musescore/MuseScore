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

#include "translation.h"

using namespace mu::inspector;

static const QMap<mu::engraving::ElementType, InspectorModelType> REST_PART_TYPES {
    { mu::engraving::ElementType::REST, InspectorModelType::TYPE_REST_BEAM }
};

RestSettingsProxyModel::RestSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_REST);
    setTitle(muse::qtrc("inspector", "Rest"));
    setIcon(muse::ui::IconCode::Code::REST_8TH);

    QList<AbstractInspectorModel*> models;
    for (InspectorModelType modelType : REST_PART_TYPES) {
        models << inspectorModelCreator()->newInspectorModel(modelType, this, repository);
    }

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated(const QList<mu::engraving::EngravingItem*>&)), this,
            SLOT(onElementsUpdated(const QList<mu::engraving::EngravingItem*>&)));
}

void RestSettingsProxyModel::onElementsUpdated(const QList<mu::engraving::EngravingItem*>& newElements)
{
    InspectorModelType defaultType = resolveDefaultSubModelType(newElements);

    setDefaultSubModelType(defaultType);
}

InspectorModelType RestSettingsProxyModel::resolveDefaultSubModelType(const QList<mu::engraving::EngravingItem*>& newElements) const
{
    InspectorModelType defaultModelType = InspectorModelType::TYPE_REST;

    for (const mu::engraving::EngravingItem* element : newElements) {
        if (REST_PART_TYPES.contains(element->type())) {
            defaultModelType = REST_PART_TYPES[element->type()];
        }
    }

    return defaultModelType;
}
