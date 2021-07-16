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
#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "text/textsettingsmodel.h"
#include "score/scoredisplaysettingsmodel.h"
#include "score/scoreappearancesettingsmodel.h"
#include "notation/inotationinteraction.h"

using namespace mu::inspector;
using namespace mu::notation;

InspectorListModel::InspectorListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(InspectorDataRole, "inspectorData");

    m_repository = new ElementRepositoryService(this);

    subscribeOnSelectionChanges();
}

void InspectorListModel::buildModelsForSelectedElements(const QSet<Ms::ElementType>& selectedElementSet)
{
    static QList<AbstractInspectorModel::InspectorSectionType> persistentSectionList
        = { AbstractInspectorModel::InspectorSectionType::SECTION_GENERAL };

    removeUnusedModels(selectedElementSet, persistentSectionList);

    QList<AbstractInspectorModel::InspectorSectionType> buildingSectionTypeList(persistentSectionList);
    bool isNotationSectionAdded = false;
    for (const Ms::ElementType elementType : selectedElementSet) {
        bool isNotationSection = AbstractInspectorModel::notationElementModelType(elementType)
                                 != AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED;
        if (isNotationSection) {
            if (!isNotationSectionAdded) {
                buildingSectionTypeList << AbstractInspectorModel::sectionTypeFromElementType(elementType);
                isNotationSectionAdded = true;
            }
        } else {
            buildingSectionTypeList << AbstractInspectorModel::sectionTypeFromElementType(elementType);
        }
    }

    createModelsBySectionType(buildingSectionTypeList, selectedElementSet);

    sortModels();
}

void InspectorListModel::buildModelsForEmptySelection(const QSet<Ms::ElementType>& selectedElementSet)
{
    static QList<AbstractInspectorModel::InspectorSectionType> persistentSectionList {
        AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_DISPLAY,
        AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_APPEARANCE
    };

    removeUnusedModels(selectedElementSet, persistentSectionList);

    createModelsBySectionType(persistentSectionList);
}

void InspectorListModel::setElementList(const QList<Ms::Element*>& selectedElementList)
{
    QSet<Ms::ElementType> newElementTypeSet;

    for (const Ms::Element* element : selectedElementList) {
        newElementTypeSet << element->type();
    }

    if (selectedElementList.isEmpty()) {
        buildModelsForEmptySelection(newElementTypeSet);
    } else {
        buildModelsForSelectedElements(newElementTypeSet);
    }

    m_repository->updateElementList(selectedElementList);

    emit modelChanged();
}

int InspectorListModel::rowCount(const QModelIndex&) const
{
    return m_modelList.count();
}

QVariant InspectorListModel::data(const QModelIndex& index, int) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty()) {
        return QVariant();
    }

    AbstractInspectorModel* model = m_modelList.at(index.row());

    QObject* result = qobject_cast<QObject*>(model);

    return QVariant::fromValue(result);
}

QHash<int, QByteArray> InspectorListModel::roleNames() const
{
    return m_roleNames;
}

int InspectorListModel::columnCount(const QModelIndex&) const
{
    return 1;
}

void InspectorListModel::createModelsBySectionType(const QList<AbstractInspectorModel::InspectorSectionType>& sectionTypeList,
                                                   const QSet<Ms::ElementType>& selectedElementSet)
{
    using SectionType = AbstractInspectorModel::InspectorSectionType;

    for (const SectionType sectionType : sectionTypeList) {
        if (sectionType == SectionType::SECTION_UNDEFINED) {
            continue;
        }

        if (isModelAlreadyExists(sectionType)) {
            continue;
        }

        beginInsertRows(QModelIndex(), rowCount(), rowCount());

        switch (sectionType) {
        case AbstractInspectorModel::InspectorSectionType::SECTION_GENERAL:
            m_modelList << new GeneralSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_TEXT:
            m_modelList << new TextSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_NOTATION:
            m_modelList << new NotationSettingsProxyModel(this, m_repository, selectedElementSet);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_DISPLAY:
            m_modelList << new ScoreSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_APPEARANCE:
            m_modelList << new ScoreAppearanceSettingsModel(this, m_repository);
            break;
        default:
            break;
        }

        endInsertRows();
    }
}

void InspectorListModel::removeUnusedModels(const QSet<Ms::ElementType>& newElementTypeSet,
                                            const QList<AbstractInspectorModel::InspectorSectionType>& exclusions)
{
    QList<AbstractInspectorModel*> modelsToRemove;

    for (AbstractInspectorModel* model : m_modelList) {
        if (exclusions.contains(model->sectionType())) {
            continue;
        }

        QList<Ms::ElementType> supportedElementTypes;
        AbstractInspectorProxyModel* proxyModel = dynamic_cast<AbstractInspectorProxyModel*>(model);
        if (proxyModel) {
            QList<Ms::ElementType> proxyElementTypes;
            for (const QVariant& modelVar : proxyModel->models()) {
                AbstractInspectorModel* prModel = qobject_cast<AbstractInspectorModel*>(modelVar.value<QObject*>());
                if (prModel) {
                    proxyElementTypes << AbstractInspectorModel::elementType(prModel->modelType());
                }
            }

            bool needRemove = false;
            for (Ms::ElementType elementType: proxyElementTypes) {
                if (!newElementTypeSet.contains(elementType)) {
                    needRemove = true;
                    break;
                }
            }

            if (!needRemove) {
                for (Ms::ElementType elementType: newElementTypeSet) {
                    if (!proxyModel->isTypeSupported(elementType)) {
                        continue;
                    }

                    if (!proxyElementTypes.contains(elementType)) {
                        needRemove = true;
                        break;
                    }
                }
            }

            if (needRemove) {
                modelsToRemove << model;
            }
        } else {
            supportedElementTypes = AbstractInspectorModel::supportedElementTypesBySectionType(model->sectionType());
            QSet<Ms::ElementType> supportedElementTypesSet(supportedElementTypes.begin(), supportedElementTypes.end());
            supportedElementTypesSet.intersect(newElementTypeSet);

            if (supportedElementTypesSet.isEmpty()) {
                modelsToRemove << model;
            }
        }
    }

    for (AbstractInspectorModel* model : modelsToRemove) {
        int index = m_modelList.indexOf(model);

        beginRemoveRows(QModelIndex(), index, index);

        delete model;
        m_modelList.removeAt(index);

        endRemoveRows();
    }
}

void InspectorListModel::sortModels()
{
    QList<AbstractInspectorModel*> sortedModelList = m_modelList;

    std::sort(sortedModelList.begin(), sortedModelList.end(), [](const AbstractInspectorModel* first,
                                                                 const AbstractInspectorModel* second) -> bool {
        return static_cast<int>(first->sectionType()) < static_cast<int>(second->sectionType());
    });

    for (int i = 0; i < m_modelList.count(); ++i) {
        if (m_modelList.at(i) != sortedModelList.at(i)) {
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), sortedModelList.indexOf(m_modelList.at(i)));
        }
    }

    if (m_modelList == sortedModelList) {
        return;
    }

    m_modelList = sortedModelList;

    endMoveRows();
}

bool InspectorListModel::isModelAlreadyExists(const AbstractInspectorModel::InspectorSectionType modelType) const
{
    for (const AbstractInspectorModel* model : m_modelList) {
        if (model->sectionType() == modelType) {
            return true;
        }
    }

    return false;
}

void InspectorListModel::subscribeOnSelectionChanges()
{
    if (!context() || !context()->currentNotation()) {
        setElementList(QList<Ms::Element*>());
    }

    context()->currentNotationChanged().onNotify(this, [this]() {
        m_notation = context()->currentNotation();

        if (!m_notation) {
            setElementList(QList<Ms::Element*>());
            return;
        }

        auto elements = m_notation->interaction()->selection()->elements();
        setElementList(QList(elements.cbegin(), elements.cend()));

        m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
            auto elements = m_notation->interaction()->selection()->elements();
            setElementList(QList(elements.cbegin(), elements.cend()));
        });
    });
}
