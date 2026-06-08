/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QHash>
#include <qqmlintegration.h>

#include "propertiespanelabstractmodel.h"

namespace mu::propertiespanel {
class PropertiesPanelAbstractProxyModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(bool isMultiModel READ isMultiModel NOTIFY modelsChanged)
    Q_PROPERTY(QVariantList models READ models NOTIFY modelsChanged)
    Q_PROPERTY(mu::propertiespanel::PropertiesPanelAbstractModel * firstModel READ firstModel NOTIFY modelsChanged)

    Q_PROPERTY(
        mu::propertiespanel::PropertiesPanelAbstractModel::PropertiesPanelModelType defaultSubModelType READ defaultSubModelType NOTIFY
        defaultSubModelTypeChanged)

public:
    explicit PropertiesPanelAbstractProxyModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                               IElementRepositoryService* repository);

    bool isMultiModel() const;
    QVariantList models() const;
    PropertiesPanelAbstractModel* firstModel() const;

    PropertiesPanelModelType defaultSubModelType() const;

    Q_INVOKABLE mu::propertiespanel::PropertiesPanelAbstractModel* modelByType(PropertiesPanelModelType type) const;

    QList<PropertiesPanelAbstractModel*> modelList() const;

    void createProperties() override {}
    void loadProperties() override {}

    void requestElements() override;
    bool isEmpty() const override;

    void updateModels(const ElementKeySet& newElementKeySet);
    bool shouldUpdateWhenEmpty() const override;
    bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const override;

    void onCurrentNotationChanged() override;

    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

public slots:
    void setDefaultSubModelType(mu::propertiespanel::PropertiesPanelModelType modelType);

signals:
    void modelsChanged();
    void defaultSubModelTypeChanged();

protected:
    void setModels(const QList<PropertiesPanelAbstractModel*>& models);

private:
    QHash<PropertiesPanelModelType, PropertiesPanelAbstractModel*> m_models;
    PropertiesPanelModelType m_defaultSubModelType = PropertiesPanelModelType::TYPE_UNDEFINED;
};
}
