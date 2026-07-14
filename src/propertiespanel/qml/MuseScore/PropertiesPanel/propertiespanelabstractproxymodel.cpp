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
#include "propertiespanelabstractproxymodel.h"

#include "propertiespanelabstractmodel.h"
#include "propertiespanelmodelfactory.h"

using namespace mu::propertiespanel;

PropertiesPanelAbstractProxyModel::PropertiesPanelAbstractProxyModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                                     IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
}

QVariantList PropertiesPanelAbstractProxyModel::models() const
{
    QVariantList objects;

    for (PropertiesPanelAbstractModel* model : modelList()) {
        objects << QVariant::fromValue(qobject_cast<QObject*>(model));
    }

    return objects;
}

PropertiesPanelAbstractModel* PropertiesPanelAbstractProxyModel::modelByType(PropertiesPanelModelType type) const
{
    return m_models.value(type);
}

PropertiesPanelAbstractModel* PropertiesPanelAbstractProxyModel::firstModel() const
{
    if (m_models.empty()) {
        return nullptr;
    }

    return m_models.values().first();
}

PropertiesPanelModelType PropertiesPanelAbstractProxyModel::defaultSubModelType() const
{
    return m_defaultSubModelType;
}

void PropertiesPanelAbstractProxyModel::setDefaultSubModelType(PropertiesPanelModelType modelType)
{
    if (m_defaultSubModelType == modelType) {
        return;
    }

    m_defaultSubModelType = modelType;
    emit defaultSubModelTypeChanged();
}

bool PropertiesPanelAbstractProxyModel::isMultiModel() const
{
    return m_models.count() > 1;
}

void PropertiesPanelAbstractProxyModel::requestElements()
{
    for (PropertiesPanelAbstractModel* model : modelList()) {
        model->requestElements();
    }
}

bool PropertiesPanelAbstractProxyModel::isEmpty() const
{
    for (const PropertiesPanelAbstractModel* model : modelList()) {
        if (!model->isEmpty()) {
            return false;
        }
    }

    return true;
}

void PropertiesPanelAbstractProxyModel::setModels(const QList<PropertiesPanelAbstractModel*>& models)
{
    QList<PropertiesPanelAbstractModel*> currentModels = modelList();

    for (PropertiesPanelAbstractModel* model : currentModels) {
        if (models.contains(model)) {
            continue;
        }

        auto oldModel = m_models.take(model->modelType());

        //! NOTE: may run synchronously from a model's own property-change callback;
        //! deleting immediately would destroy "this" mid-call, so defer it
        oldModel->deleteLater();
    }

    for (PropertiesPanelAbstractModel* model : models) {
        if (!model) {
            continue;
        }

        PropertiesPanelModelType modelType = model->modelType();

        if (m_models.contains(modelType)) {
            continue;
        }

        connect(model, &PropertiesPanelAbstractModel::isEmptyChanged, this, [this]() {
            emit isEmptyChanged();
        });

        connect(model, &PropertiesPanelAbstractModel::requestReloadPropertiesPanelListModel, this,
                &PropertiesPanelAbstractModel::requestReloadPropertiesPanelListModel);

        m_models[modelType] = model;
    }

    emit modelsChanged();
}

void PropertiesPanelAbstractProxyModel::onCurrentNotationChanged()
{
    for (PropertiesPanelAbstractModel* model : modelList()) {
        model->onCurrentNotationChanged();
    }

    PropertiesPanelAbstractModel::onCurrentNotationChanged();
}

void PropertiesPanelAbstractProxyModel::onNotationChanged(const engraving::PropertyIdSet& changedPropertyIdSet,
                                                          const engraving::StyleIdSet& changedStyleIdSet)
{
    for (PropertiesPanelAbstractModel* model : modelList()) {
        if (!model->shouldUpdateOnScoreChange()) {
            continue;
        }

        if (!model->shouldUpdateWhenEmpty() && model->isEmpty()) {
            continue;
        }

        if (!model->shouldUpdateOnEmptyPropertyAndStyleIdSets()) {
            if (changedPropertyIdSet.empty() && changedStyleIdSet.empty()) {
                continue;
            }
        }

        mu::engraving::PropertyIdSet expandedPropertyIdSet = model->propertyIdSetFromStyleIdSet(changedStyleIdSet);
        expandedPropertyIdSet.insert(changedPropertyIdSet.cbegin(), changedPropertyIdSet.cend());
        model->onNotationChanged(expandedPropertyIdSet, changedStyleIdSet);
    }

    PropertiesPanelAbstractModel::onNotationChanged(changedPropertyIdSet, changedStyleIdSet);
}

void PropertiesPanelAbstractProxyModel::updateModels(const ElementKeySet& newElementKeySet)
{
    QList<PropertiesPanelAbstractModel*> models;

    for (const ElementKey& elementKey : newElementKeySet) {
        PropertiesPanelModelType modelType = PropertiesPanelAbstractModel::modelTypeByElementKey(elementKey);

        if (modelType == PropertiesPanelModelType::TYPE_UNDEFINED) {
            continue;
        }

        auto model = dynamic_cast<PropertiesPanelAbstractModel*>(modelByType(modelType));

        if (!model) {
            model = PropertiesPanelModelFactory::newPropertiesPanelListModel(modelType, this, iocContext(), m_repository);
        }

        if (model) {
            models << model;
        }
    }

    setModels(models);
}

bool PropertiesPanelAbstractProxyModel::shouldUpdateWhenEmpty() const
{
    for (const PropertiesPanelAbstractModel* model : modelList()) {
        if (model->shouldUpdateWhenEmpty()) {
            return true;
        }
    }
    return false;
}

bool PropertiesPanelAbstractProxyModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    for (const PropertiesPanelAbstractModel* model : modelList()) {
        if (model->shouldUpdateOnEmptyPropertyAndStyleIdSets()) {
            return true;
        }
    }
    return false;
}

QList<PropertiesPanelAbstractModel*> PropertiesPanelAbstractProxyModel::modelList() const
{
    return m_models.values();
}
