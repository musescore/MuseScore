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

AbstractInspectorProxyModel::AbstractInspectorProxyModel(QObject* parent)
    : AbstractInspectorModel(parent)
{
}

QObject* AbstractInspectorProxyModel::modelByType(const InspectorModelType type)
{
    return m_modelsHash.value(static_cast<int>(type));
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
