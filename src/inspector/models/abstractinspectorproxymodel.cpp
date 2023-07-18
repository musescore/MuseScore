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
#include "abstractinspectorproxymodel.h"

using namespace mu::inspector;

AbstractInspectorProxyModel::AbstractInspectorProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
}

QVariantList AbstractInspectorProxyModel::models() const
{
    QVariantList objects;

    for (AbstractInspectorModel* model : modelList()) {
        objects << QVariant::fromValue(qobject_cast<QObject*>(model));
    }

    return objects;
}

QObject* AbstractInspectorProxyModel::modelByType(InspectorModelType type) const
{
    return m_modelsHash.value(type);
}

QObject* AbstractInspectorProxyModel::firstModel() const
{
    if (m_modelsHash.empty()) {
        return nullptr;
    }

    return m_modelsHash.values().first();
}

InspectorModelType AbstractInspectorProxyModel::defaultSubModelType() const
{
    return m_defaultSubModelType;
}

void AbstractInspectorProxyModel::setDefaultSubModelType(InspectorModelType modelType)
{
    if (m_defaultSubModelType == modelType) {
        return;
    }

    m_defaultSubModelType = modelType;
    emit defaultSubModelTypeChanged();
}

bool AbstractInspectorProxyModel::isMultiModel() const
{
    return m_modelsHash.count() > 1;
}

void AbstractInspectorProxyModel::requestElements()
{
    for (AbstractInspectorModel* model : modelList()) {
        model->requestElements();
    }
}

void AbstractInspectorProxyModel::requestResetToDefaults()
{
    for (AbstractInspectorModel* model : modelList()) {
        model->requestResetToDefaults();
    }
}

bool AbstractInspectorProxyModel::isEmpty() const
{
    for (const AbstractInspectorModel* model : modelList()) {
        if (!model->isEmpty()) {
            return false;
        }
    }

    return true;
}

void AbstractInspectorProxyModel::setModels(const QList<AbstractInspectorModel*>& models)
{
    QList<AbstractInspectorModel*> currentModels = modelList();

    for (AbstractInspectorModel* model : currentModels) {
        if (models.contains(model)) {
            continue;
        }

        auto oldModel = m_modelsHash.take(model->modelType());

        delete oldModel;
        oldModel = nullptr;
    }

    for (AbstractInspectorModel* model : models) {
        if (!model) {
            continue;
        }

        InspectorModelType modelType = model->modelType();

        if (m_modelsHash.contains(modelType)) {
            continue;
        }

        connect(model, &AbstractInspectorModel::isEmptyChanged, this, [this]() {
            emit isEmptyChanged();
        });

        m_modelsHash[modelType] = model;
    }

    emit modelsChanged();
}

void AbstractInspectorProxyModel::onCurrentNotationChanged()
{
    for (AbstractInspectorModel* model : modelList()) {
        model->onCurrentNotationChanged();
    }

    AbstractInspectorModel::onCurrentNotationChanged();
}

void AbstractInspectorProxyModel::updateModels(const ElementKeySet& newElementKeySet)
{
    QList<AbstractInspectorModel*> models;

    for (const ElementKey& elementKey : newElementKeySet) {
        InspectorModelType modelType = AbstractInspectorModel::modelTypeByElementKey(elementKey);

        if (modelType == InspectorModelType::TYPE_UNDEFINED) {
            continue;
        }

        auto model = dynamic_cast<AbstractInspectorModel*>(modelByType(modelType));

        if (!model) {
            model = inspectorModelCreator()->newInspectorModel(modelType, this, m_repository);
        }

        if (model) {
            models << model;
        }
    }

    setModels(models);
}

QList<AbstractInspectorModel*> AbstractInspectorProxyModel::modelList() const
{
    return m_modelsHash.values();
}
