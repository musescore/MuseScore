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

#include <memory>

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/select.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "propertiespanelabstractmodel.h"

namespace mu::propertiespanel {
class IElementRepositoryService;
class PropertiesPanelListModel : public QAbstractListModel, public QQmlParserStatus, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> context = { this };

public:
    explicit PropertiesPanelListModel(QObject* parent = nullptr);
    ~PropertiesPanelListModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE void setPropertiesPanelVisible(bool visible);

private:
    enum RoleNames {
        PropertiesPanelSectionModelRole = Qt::UserRole + 1
    };

    void classBegin() override;
    void componentComplete() override {}
    void init();

    void listenSelectionChanged();
    void listenScoreChanges();

    void onScoreChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet, const mu::engraving::StyleIdSet& changedStyleIdSet);

    void updateElementList();

    bool alwaysUpdateModelList(const QList<mu::engraving::EngravingItem*>& selectedElementList);
    void setElementList(const QList<mu::engraving::EngravingItem*>& selectedElementList,
                        engraving::SelState selectionState = engraving::SelState::NONE);

    void buildModelsForEmptySelection();
    void buildModelsForSelectedElements(const ElementKeySet& selectedElementKeySet, bool isRangeSelection,
                                        const QList<engraving::EngravingItem*>& selectedElementList);

    void createModelsBySectionType(const PropertiesPanelSectionTypeSet& sectionTypes, const ElementKeySet& selectedElementKeySet = {});
    void removeUnusedModels(const ElementKeySet& newElementKeySet, bool isRangeSelection,
                            const QList<mu::engraving::EngravingItem*>& selectedElementList,
                            const PropertiesPanelSectionTypeSet& exclusions = {});

    bool isModelAllowed(const PropertiesPanelAbstractModel* model, const PropertiesPanelModelTypeSet& allowedModelTypes,
                        const PropertiesPanelSectionTypeSet& allowedSectionTypes) const;

    void sortModels();

    PropertiesPanelAbstractModel* modelBySectionType(PropertiesPanelSectionType sectionType) const;

    void notifyModelsAboutNotationChanged();

    QList<PropertiesPanelAbstractModel*> m_modelList;

    std::unique_ptr<IElementRepositoryService> m_repository;

    bool m_propertiespanelVisible = true;
    mu::engraving::PropertyIdSet m_changedPropertyIdSet;
    mu::engraving::StyleIdSet m_changedStyleIdSet;
};
}
