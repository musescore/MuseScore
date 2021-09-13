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

AbstractInspectorProxyModel::AbstractInspectorProxyModel(QObject* parent, IElementRepositoryService* repository,
                                                         InspectorModelType preferedSubModelType)
    : AbstractInspectorModel(parent, repository), m_preferedSubModelType(preferedSubModelType)
{
}

QObject* AbstractInspectorProxyModel::modelByType(const InspectorModelType type)
{
    return m_modelsHash.value(static_cast<int>(type));
}

QVariantList AbstractInspectorProxyModel::models()
{
    QVariantList objects;

    for (AbstractInspectorModel* model : modelList()) {
        objects << QVariant::fromValue(qobject_cast<QObject*>(model));
    }

    return objects;
}

bool AbstractInspectorProxyModel::isMultiModel() const
{
    return m_modelsHash.count() > 1;
}

QObject* AbstractInspectorProxyModel::firstModel()
{
    if (m_modelsHash.empty()) {
        return nullptr;
    }

    return m_modelsHash.values().first();
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

AbstractInspectorModel::InspectorModelType AbstractInspectorProxyModel::preferedSubModelType() const
{
    return m_preferedSubModelType;
}

void AbstractInspectorProxyModel::addModel(AbstractInspectorModel* model)
{
    if (!model) {
        return;
    }

    connect(model, &AbstractInspectorModel::isEmptyChanged, this, [this]() {
        emit isEmptyChanged();
    });

    m_modelsHash.insert(static_cast<int>(model->modelType()), model);
}

QList<AbstractInspectorModel*> AbstractInspectorProxyModel::modelList() const
{
    return m_modelsHash.values();
}
