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

AbstractInspectorProxyModel::AbstractInspectorProxyModel(QObject* parent, InspectorModelType preferedSubModelType)
    : AbstractInspectorModel(parent), m_preferedSubModelType(preferedSubModelType)
{
}

QObject* AbstractInspectorProxyModel::modelByType(const InspectorModelType type)
{
    return m_modelsHash.value(static_cast<int>(type));
}

QVariantList AbstractInspectorProxyModel::models()
{
    QVariantList objects;

    for (AbstractInspectorModel* model : m_modelsHash.values()) {
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

void AbstractInspectorProxyModel::requestResetToDefaults()
{
    for (AbstractInspectorModel* model : m_modelsHash.values()) {
        model->requestResetToDefaults();
    }
}

bool AbstractInspectorProxyModel::hasAcceptableElements() const
{
    bool result = false;

    for (const AbstractInspectorModel* model : m_modelsHash.values()) {
        result |= model->hasAcceptableElements();
    }

    return result;
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
        setIsEmpty(!hasAcceptableElements());
    });

    m_modelsHash.insert(static_cast<int>(model->modelType()), model);
}
