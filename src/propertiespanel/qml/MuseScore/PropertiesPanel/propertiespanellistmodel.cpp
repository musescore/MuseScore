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

#include "propertiespanellistmodel.h"

#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationselection.h"
#include "notation/inotationundostack.h" // IWYU pragma: keep

#include "engraving/dom/score.h"

#include "general/generalsettingsmodel.h"
#include "systemlayout/systemlayoutsettingsmodel.h"
#include "measures/measuressettingsmodel.h"
#include "emptystaves/emptystavesvisiblitysettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "parts/partssettingsmodel.h"
#include "text/textsettingsmodel.h"
#include "score/scoredisplaysettingsmodel.h"
#include "score/scoreappearancesettingsmodel.h"

#include "internal/elementrepositoryservice.h"

#include "log.h"

using namespace mu::propertiespanel;
using namespace mu::notation;

PropertiesPanelListModel::PropertiesPanelListModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
    , m_repository{std::make_unique<ElementRepositoryService>()}
{
}

PropertiesPanelListModel::~PropertiesPanelListModel() = default;

void PropertiesPanelListModel::classBegin()
{
    init();
}

void PropertiesPanelListModel::init()
{
    listenSelectionChanged();
    listenScoreChanges();

    context()->currentNotationChanged().onNotify(this, [this]() {
        listenSelectionChanged();
        listenScoreChanges();

        notifyModelsAboutNotationChanged();
    });
}

void PropertiesPanelListModel::buildModelsForSelectedElements(const ElementKeySet& selectedElementKeySet, bool isRangeSelection,
                                                              const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    removeUnusedModels(selectedElementKeySet, isRangeSelection, selectedElementList);

    PropertiesPanelSectionTypeSet buildingSectionTypeSet = PropertiesPanelAbstractModel::sectionTypesByElementKeys(selectedElementKeySet,
                                                                                                                   isRangeSelection,
                                                                                                                   selectedElementList);

    createModelsBySectionType(buildingSectionTypeSet, selectedElementKeySet);

    sortModels();
}

void PropertiesPanelListModel::buildModelsForEmptySelection()
{
    if (context()->currentNotation() == nullptr) {
        removeUnusedModels({}, false /*isRangeSelection*/, {});
        return;
    }

    static const PropertiesPanelSectionTypeSet persistentSections {
        PropertiesPanelSectionType::SECTION_SCORE_DISPLAY,
        PropertiesPanelSectionType::SECTION_SCORE_APPEARANCE
    };

    removeUnusedModels({}, false /*isRangeSelection*/, {}, persistentSections);

    createModelsBySectionType(persistentSections);
}

bool PropertiesPanelListModel::alwaysUpdateModelList(const QList<engraving::EngravingItem*>& selectedElementList)
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

void PropertiesPanelListModel::setElementList(const QList<mu::engraving::EngravingItem*>& selectedElementList,
                                              engraving::SelState selectionState)
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
            newElementKeySet << PropertiesPanelAbstractModel::makeKey(element);
        }

        buildModelsForSelectedElements(newElementKeySet, selectionState == SelectionState::RANGE, selectedElementList);
    }

    m_repository->updateElementList(selectedElementList, selectionState);

    if (forceUpdate) {
        m_repository->elementsUpdated().send(selectedElementList);
    }
}

int PropertiesPanelListModel::rowCount(const QModelIndex&) const
{
    return m_modelList.count();
}

QVariant PropertiesPanelListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty() || role != PropertiesPanelSectionModelRole) {
        return QVariant();
    }

    PropertiesPanelAbstractModel* model = m_modelList.at(index.row());

    QObject* result = qobject_cast<QObject*>(model);
    assert(result != nullptr);

    return QVariant::fromValue(result);
}

QHash<int, QByteArray> PropertiesPanelListModel::roleNames() const
{
    return {
        { PropertiesPanelSectionModelRole, "propertiesPanelSectionModel" }
    };
}

int PropertiesPanelListModel::columnCount(const QModelIndex&) const
{
    return 1;
}

void PropertiesPanelListModel::setPropertiesPanelVisible(bool visible)
{
    if (m_propertiespanelVisible == visible) {
        return;
    }

    m_propertiespanelVisible = visible;

    if (visible) {
        updateElementList();

        if (!m_changedPropertyIdSet.empty() || !m_changedStyleIdSet.empty()) {
            onScoreChanged(m_changedPropertyIdSet, m_changedStyleIdSet);

            m_changedPropertyIdSet.clear();
            m_changedStyleIdSet.clear();
        }
    }
}

void PropertiesPanelListModel::createModelsBySectionType(const PropertiesPanelSectionTypeSet& sectionTypes,
                                                         const ElementKeySet& selectedElementKeySet)
{
    for (PropertiesPanelSectionType sectionType : sectionTypes) {
        if (sectionType == PropertiesPanelSectionType::SECTION_UNDEFINED) {
            continue;
        }

        PropertiesPanelAbstractModel* model = modelBySectionType(sectionType);

        if (model) {
            if (auto proxyModel = dynamic_cast<PropertiesPanelAbstractProxyModel*>(model)) {
                proxyModel->updateModels(selectedElementKeySet);
            }

            continue;
        }

        int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);

        PropertiesPanelAbstractModel* newModel = nullptr;

        switch (sectionType) {
        case PropertiesPanelSectionType::SECTION_GENERAL:
            newModel = new GeneralSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_MEASURES:
            newModel = new MeasuresSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_SYSTEM_LAYOUT:
            newModel = new SystemLayoutSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_EMPTY_STAVES:
            newModel = new EmptyStavesVisibilitySettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_NOTATION:
            newModel = new NotationSettingsProxyModel(this, iocContext(), m_repository.get(), selectedElementKeySet);
            break;
        case PropertiesPanelSectionType::SECTION_TEXT:
            newModel = new TextSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_SCORE_DISPLAY:
            newModel = new ScoreDisplaySettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_SCORE_APPEARANCE:
            newModel = new ScoreAppearanceSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelSectionType::SECTION_PARTS:
            newModel = new PartsSettingsModel(this, iocContext(), m_repository.get());
            break;
        case PropertiesPanelAbstractModel::PropertiesPanelSectionType::SECTION_UNDEFINED:
            break;
        }

        if (newModel) {
            connect(newModel, &PropertiesPanelAbstractModel::requestReloadPropertiesPanelListModel, this,
                    &PropertiesPanelListModel::updateElementList);
            newModel->init();
            m_modelList << newModel;
        }

        endInsertRows();
    }
}

void PropertiesPanelListModel::removeUnusedModels(const ElementKeySet& newElementKeySet,
                                                  bool isRangeSelection, const QList<engraving::EngravingItem*>& selectedElementList,
                                                  const PropertiesPanelSectionTypeSet& exclusions)
{
    QList<PropertiesPanelAbstractModel*> modelsToRemove;

    PropertiesPanelModelTypeSet allowedModelTypes = PropertiesPanelAbstractModel::modelTypesByElementKeys(newElementKeySet);
    PropertiesPanelSectionTypeSet allowedSectionTypes = PropertiesPanelAbstractModel::sectionTypesByElementKeys(newElementKeySet,
                                                                                                                isRangeSelection,
                                                                                                                selectedElementList);

    for (PropertiesPanelAbstractModel* model : m_modelList) {
        if (muse::contains(exclusions, model->sectionType())) {
            continue;
        }

        if (!isModelAllowed(model, allowedModelTypes, allowedSectionTypes)) {
            modelsToRemove << model;
        }
    }

    for (PropertiesPanelAbstractModel* model : modelsToRemove) {
        int index = m_modelList.indexOf(model);

        beginRemoveRows(QModelIndex(), index, index);

        m_modelList.removeAt(index);

        delete model;
        model = nullptr;

        endRemoveRows();
    }
}

bool PropertiesPanelListModel::isModelAllowed(const PropertiesPanelAbstractModel* model,
                                              const PropertiesPanelModelTypeSet& allowedModelTypes,
                                              const PropertiesPanelSectionTypeSet& allowedSectionTypes) const
{
    PropertiesPanelModelType modelType = model->modelType();

    if (modelType != PropertiesPanelModelType::TYPE_UNDEFINED && muse::contains(allowedModelTypes, modelType)) {
        return true;
    }

    auto proxyModel = dynamic_cast<const PropertiesPanelAbstractProxyModel*>(model);
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

void PropertiesPanelListModel::sortModels()
{
    QList<PropertiesPanelAbstractModel*> sortedModelList = m_modelList;

    std::sort(sortedModelList.begin(), sortedModelList.end(), [](const PropertiesPanelAbstractModel* first,
                                                                 const PropertiesPanelAbstractModel* second) -> bool {
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

PropertiesPanelAbstractModel* PropertiesPanelListModel::modelBySectionType(PropertiesPanelSectionType sectionType) const
{
    for (PropertiesPanelAbstractModel* model : m_modelList) {
        if (model->sectionType() == sectionType) {
            return model;
        }
    }

    return nullptr;
}

void PropertiesPanelListModel::notifyModelsAboutNotationChanged()
{
    TRACEFUNC;

    for (PropertiesPanelAbstractModel* model : m_modelList) {
        model->onCurrentNotationChanged();
    }
}

void PropertiesPanelListModel::listenSelectionChanged()
{
    updateElementList();

    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateElementList();
    }, Asyncable::Mode::SetReplace /* FIXME */);
}

void PropertiesPanelListModel::listenScoreChanges()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    notation->viewModeChanged().onNotify(this, [this]() {
        for (PropertiesPanelAbstractModel* model : m_modelList) {
            model->onNotationChanged({}, {});
        }
    }, Asyncable::Mode::SetReplace /* FIXME */);

    notation->undoStack()->changesChannel().onReceive(this, [this](const engraving::ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (!m_propertiespanelVisible) {
            m_changedPropertyIdSet.insert(changes.changedPropertyIdSet.cbegin(), changes.changedPropertyIdSet.cend());
            m_changedStyleIdSet.insert(changes.changedStyleIdSet.cbegin(), changes.changedStyleIdSet.cend());
            return;
        }

        const INotationPtr notation = context()->currentNotation();
        if (notation && notation->elements()->msScore()->selectionChanged()) {
            updateElementList();
        }

        onScoreChanged(changes.changedPropertyIdSet, changes.changedStyleIdSet);
    }, Asyncable::Mode::SetReplace /* FIXME */);
}

void PropertiesPanelListModel::onScoreChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                              const mu::engraving::StyleIdSet& changedStyleIdSet)
{
    for (PropertiesPanelAbstractModel* model : m_modelList) {
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
}

void PropertiesPanelListModel::updateElementList()
{
    if (!m_propertiespanelVisible) {
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
