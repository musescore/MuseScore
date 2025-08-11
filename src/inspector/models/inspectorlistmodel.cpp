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
#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "measures/measuressettingsmodel.h"
#include "emptystaves/emptystavesvisiblitysettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "parts/partssettingsmodel.h"
#include "text/textsettingsmodel.h"
#include "score/scoredisplaysettingsmodel.h"
#include "score/scoreappearancesettingsmodel.h"
#include "notation/inotationinteraction.h"

#include "internal/services/elementrepositoryservice.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;

InspectorListModel::InspectorListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_repository = new ElementRepositoryService(this);

    listenSelectionChanged();
    listenScoreChanges();

    context()->currentNotationChanged().onNotify(this, [this]() {
        listenSelectionChanged();
        listenScoreChanges();

        notifyModelsAboutNotationChanged();
    });
}

void InspectorListModel::buildModelsForSelectedElements(const ElementKeySet& selectedElementKeySet, bool isRangeSelection,
                                                        const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    removeUnusedModels(selectedElementKeySet, isRangeSelection, selectedElementList);

    InspectorSectionTypeSet buildingSectionTypeSet = AbstractInspectorModel::sectionTypesByElementKeys(selectedElementKeySet,
                                                                                                       isRangeSelection,
                                                                                                       selectedElementList);

    createModelsBySectionType(buildingSectionTypeSet, selectedElementKeySet);

    sortModels();
}

void InspectorListModel::buildModelsForEmptySelection()
{
    if (context()->currentNotation() == nullptr) {
        removeUnusedModels({}, false /*isRangeSelection*/, {});
        return;
    }

    static const InspectorSectionTypeSet persistentSections {
        InspectorSectionType::SECTION_SCORE_DISPLAY,
        InspectorSectionType::SECTION_SCORE_APPEARANCE
    };

    removeUnusedModels({}, false /*isRangeSelection*/, {}, persistentSections);

    createModelsBySectionType(persistentSections);
}

bool InspectorListModel::alwaysUpdateModelList(const QList<engraving::EngravingItem*>& selectedElementList)
{
    // Force update of the list model where sections are only relevant to the child of the selected element
    // eg. We need to update the text section of PlayCountText when a BarLine is selected
    for (EngravingItem* el : selectedElementList) {
        if (el->isBarLine()) {
            return true;
        }
    }

    return false;
}

void InspectorListModel::setElementList(const QList<mu::engraving::EngravingItem*>& selectedElementList, SelectionState selectionState)
{
    TRACEFUNC;

    bool forceUpdate = false;

    if (!m_modelList.isEmpty()) {
        if (context()->currentNotation() == nullptr) {
            buildModelsForEmptySelection();
        }

        forceUpdate = alwaysUpdateModelList(selectedElementList);
        if (!m_repository->needUpdateElementList(selectedElementList, selectionState) && !forceUpdate) {
            return;
        }
    }

    if (selectedElementList.isEmpty()) {
        buildModelsForEmptySelection();
    } else {
        ElementKeySet newElementKeySet;

        for (const mu::engraving::EngravingItem* element : selectedElementList) {
            newElementKeySet << AbstractInspectorModel::makeKey(element);
        }

        buildModelsForSelectedElements(newElementKeySet, selectionState == SelectionState::RANGE, selectedElementList);
    }

    m_repository->updateElementList(selectedElementList, selectionState);

    if (forceUpdate) {
        m_repository->elementsUpdated(selectedElementList);
    }
}

int InspectorListModel::rowCount(const QModelIndex&) const
{
    return m_modelList.count();
}

QVariant InspectorListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty() || role != InspectorSectionModelRole) {
        return QVariant();
    }

    AbstractInspectorModel* model = m_modelList.at(index.row());

    QObject* result = qobject_cast<QObject*>(model);

    return QVariant::fromValue(result);
}

QHash<int, QByteArray> InspectorListModel::roleNames() const
{
    return {
        { InspectorSectionModelRole, "inspectorSectionModel" }
    };
}

int InspectorListModel::columnCount(const QModelIndex&) const
{
    return 1;
}

void InspectorListModel::setInspectorVisible(bool visible)
{
    if (m_inspectorVisible == visible) {
        return;
    }

    m_inspectorVisible = visible;

    if (visible) {
        updateElementList();

        if (!m_changedPropertyIdSet.empty() || !m_changedStyleIdSet.empty()) {
            onScoreChanged(m_changedPropertyIdSet, m_changedStyleIdSet);

            m_changedPropertyIdSet.clear();
            m_changedStyleIdSet.clear();
        }
    }
}

void InspectorListModel::createModelsBySectionType(const InspectorSectionTypeSet& sectionTypes,
                                                   const ElementKeySet& selectedElementKeySet)
{
    for (InspectorSectionType sectionType : sectionTypes) {
        if (sectionType == InspectorSectionType::SECTION_UNDEFINED) {
            continue;
        }

        AbstractInspectorModel* model = modelBySectionType(sectionType);

        if (model) {
            if (auto proxyModel = dynamic_cast<AbstractInspectorProxyModel*>(model)) {
                proxyModel->updateModels(selectedElementKeySet);
            }

            continue;
        }

        int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);

        AbstractInspectorModel* newModel = nullptr;

        switch (sectionType) {
        case InspectorSectionType::SECTION_GENERAL:
            newModel = new GeneralSettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_MEASURES:
            newModel = new MeasuresSettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_EMPTY_STAVES:
            newModel = new EmptyStavesVisibilitySettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_NOTATION:
            newModel = new NotationSettingsProxyModel(this, m_repository, selectedElementKeySet);
            break;
        case InspectorSectionType::SECTION_TEXT:
            newModel = new TextSettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_SCORE_DISPLAY:
            newModel = new ScoreSettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_SCORE_APPEARANCE:
            newModel = new ScoreAppearanceSettingsModel(this, m_repository);
            break;
        case InspectorSectionType::SECTION_PARTS:
            newModel = new PartsSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_UNDEFINED:
            break;
        }

        if (newModel) {
            connect(newModel, &AbstractInspectorModel::requestReloadInspectorListModel, this,
                    &InspectorListModel::updateElementList);
            newModel->init();
            m_modelList << newModel;
        }

        endInsertRows();
    }
}

void InspectorListModel::removeUnusedModels(const ElementKeySet& newElementKeySet,
                                            bool isRangeSelection, const QList<engraving::EngravingItem*>& selectedElementList,
                                            const InspectorSectionTypeSet& exclusions)
{
    QList<AbstractInspectorModel*> modelsToRemove;

    InspectorModelTypeSet allowedModelTypes = AbstractInspectorModel::modelTypesByElementKeys(newElementKeySet);
    InspectorSectionTypeSet allowedSectionTypes = AbstractInspectorModel::sectionTypesByElementKeys(newElementKeySet, isRangeSelection,
                                                                                                    selectedElementList);

    for (AbstractInspectorModel* model : m_modelList) {
        if (muse::contains(exclusions, model->sectionType())) {
            continue;
        }

        if (!isModelAllowed(model, allowedModelTypes, allowedSectionTypes)) {
            modelsToRemove << model;
        }
    }

    for (AbstractInspectorModel* model : modelsToRemove) {
        int index = m_modelList.indexOf(model);

        beginRemoveRows(QModelIndex(), index, index);

        m_modelList.removeAt(index);

        delete model;
        model = nullptr;

        endRemoveRows();
    }
}

bool InspectorListModel::isModelAllowed(const AbstractInspectorModel* model, const InspectorModelTypeSet& allowedModelTypes,
                                        const InspectorSectionTypeSet& allowedSectionTypes) const
{
    InspectorModelType modelType = model->modelType();

    if (modelType != InspectorModelType::TYPE_UNDEFINED && muse::contains(allowedModelTypes, modelType)) {
        return true;
    }

    auto proxyModel = dynamic_cast<const AbstractInspectorProxyModel*>(model);
    if (!proxyModel) {
        return muse::contains(allowedSectionTypes, model->sectionType());
    }

    for (auto subModel : proxyModel->modelList()) {
        if (isModelAllowed(subModel, allowedModelTypes, allowedSectionTypes)) {
            return true;
        }
    }

    return false;
}

void InspectorListModel::sortModels()
{
    QList<AbstractInspectorModel*> sortedModelList = m_modelList;

    std::sort(sortedModelList.begin(), sortedModelList.end(), [](const AbstractInspectorModel* first,
                                                                 const AbstractInspectorModel* second) -> bool {
        return static_cast<int>(first->sectionType()) < static_cast<int>(second->sectionType());
    });

    if (sortedModelList == m_modelList) {
        return;
    }

    for (int i = 0; i < m_modelList.count(); ++i) {
        if (m_modelList[i] != sortedModelList[i]) {
            m_modelList[i] = sortedModelList[i];
            emit dataChanged(index(i, 0), index(i, 0));
        }
    }
}

AbstractInspectorModel* InspectorListModel::modelBySectionType(InspectorSectionType sectionType) const
{
    for (AbstractInspectorModel* model : m_modelList) {
        if (model->sectionType() == sectionType) {
            return model;
        }
    }

    return nullptr;
}

void InspectorListModel::notifyModelsAboutNotationChanged()
{
    TRACEFUNC;

    for (AbstractInspectorModel* model : m_modelList) {
        model->onCurrentNotationChanged();
    }
}

void InspectorListModel::listenSelectionChanged()
{
    updateElementList();

    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateElementList();
    });
}

void InspectorListModel::listenScoreChanges()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    notation->viewModeChanged().onNotify(this, [this]() {
        for (AbstractInspectorModel* model : m_modelList) {
            model->onNotationChanged({}, {});
        }
    });

    notation->undoStack()->changesChannel().onReceive(this, [this](const ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (changes.changedPropertyIdSet.empty() && changes.changedStyleIdSet.empty()) {
            return;
        }

        if (!m_inspectorVisible) {
            m_changedPropertyIdSet.insert(changes.changedPropertyIdSet.cbegin(), changes.changedPropertyIdSet.cend());
            m_changedStyleIdSet.insert(changes.changedStyleIdSet.cbegin(), changes.changedStyleIdSet.cend());
            return;
        }

        const INotationPtr notation = context()->currentNotation();
        if (notation && notation->elements()->msScore()->selectionChanged()) {
            updateElementList();
        }

        onScoreChanged(changes.changedPropertyIdSet, changes.changedStyleIdSet);
    });
}

void InspectorListModel::onScoreChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                        const mu::engraving::StyleIdSet& changedStyleIdSet)
{
    for (AbstractInspectorModel* model : m_modelList) {
        if (!model->shouldUpdateOnScoreChange() || model->isEmpty()) {
            continue;
        }

        mu::engraving::PropertyIdSet expandedPropertyIdSet = model->propertyIdSetFromStyleIdSet(changedStyleIdSet);
        expandedPropertyIdSet.insert(changedPropertyIdSet.cbegin(), changedPropertyIdSet.cend());
        model->onNotationChanged(expandedPropertyIdSet, changedStyleIdSet);
    }
}

void InspectorListModel::updateElementList()
{
    if (!m_inspectorVisible) {
        return;
    }

    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        setElementList({});
        return;
    }

    INotationSelectionPtr selection = notation->interaction()->selection();
    const std::vector<EngravingItem*>& elements = selection->elements();
    setElementList(QList(elements.cbegin(), elements.cend()), selection->state());
}
