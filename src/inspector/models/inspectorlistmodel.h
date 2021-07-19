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
#ifndef MU_INSPECTOR_INSPECTORLISTMODEL_H
#define MU_INSPECTOR_INSPECTORLISTMODEL_H

#include <QAbstractListModel>

#include "libmscore/element.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "internal/services/elementrepositoryservice.h"
#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class InspectorListModel : public QAbstractListModel, public mu::async::Asyncable
{
    Q_OBJECT

    INJECT(inspector, context::IGlobalContext, context)

public:
    explicit InspectorListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

signals:
    void elementsModified();
    void modelChanged();

private:
    enum RoleNames {
        InspectorDataRole = Qt::UserRole + 1,
        InspectorTitleRole
    };

    void setElementList(const QList<Ms::Element*>& selectedElementList);

    void buildModelsForEmptySelection(const QSet<Ms::ElementType>& selectedElementSet);
    void buildModelsForSelectedElements(const QSet<Ms::ElementType>& selectedElementSet);

    void createModelsBySectionType(const QList<AbstractInspectorModel::InspectorSectionType>& sectionTypeList,
                                   const QSet<Ms::ElementType>& selectedElementSet = {});
    void removeUnusedModels(const QSet<Ms::ElementType>& newElementTypeSet,
                            const QList<AbstractInspectorModel::InspectorSectionType>& exclusions = QList<AbstractInspectorModel::InspectorSectionType>());

    void sortModels();

    bool isModelAlreadyExists(const AbstractInspectorModel::InspectorSectionType modelType) const;

    void subscribeOnSelectionChanges();

    QHash<int, QByteArray> m_roleNames;
    QList<AbstractInspectorModel*> m_modelList;

    IElementRepositoryService* m_repository = nullptr;
    notation::INotationPtr m_notation;
};
}

#endif // MU_INSPECTOR_INSPECTORLISTMODEL_H
