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
#ifndef MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H
#define MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H

#include "models/abstractinspectormodel.h"
#include "models/iinspectormodelcreator.h"

#include <QHash>

namespace mu::inspector {
class AbstractInspectorProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(bool isMultiModel READ isMultiModel NOTIFY modelsChanged)
    Q_PROPERTY(QVariantList models READ models NOTIFY modelsChanged)
    Q_PROPERTY(QObject * firstModel READ firstModel NOTIFY modelsChanged)

    Q_PROPERTY(InspectorModelType defaultSubModelType READ defaultSubModelType NOTIFY defaultSubModelTypeChanged)

public:
    INJECT(IInspectorModelCreator, inspectorModelCreator)

public:
    explicit AbstractInspectorProxyModel(QObject* parent, IElementRepositoryService* repository);

    bool isMultiModel() const;
    QVariantList models() const;
    QObject* firstModel() const;

    InspectorModelType defaultSubModelType() const;

    Q_INVOKABLE QObject* modelByType(InspectorModelType type) const;

    QList<AbstractInspectorModel*> modelList() const;

    void createProperties() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    void requestElements() override;
    void requestResetToDefaults() override;
    bool isEmpty() const override;

    void updateModels(const ElementKeySet& newElementKeySet);

    void onCurrentNotationChanged() override;

public slots:
    void setDefaultSubModelType(mu::inspector::InspectorModelType modelType);

signals:
    void modelsChanged();
    void defaultSubModelTypeChanged();

protected:
    void setModels(const QList<AbstractInspectorModel*>& models);

private:
    QHash<InspectorModelType, AbstractInspectorModel*> m_models;
    InspectorModelType m_defaultSubModelType = InspectorModelType::TYPE_UNDEFINED;
};
}

#endif // MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H
