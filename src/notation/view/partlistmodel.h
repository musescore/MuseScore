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

#ifndef MU_NOTATION_PARTLISTMODEL_H
#define MU_NOTATION_PARTLISTMODEL_H

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"

namespace mu::uicomponents {
class ItemMultiSelectionModel;
}

namespace mu::notation {
class PartListModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, framework::IInteractive, interactive)

    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY selectionChanged)

public:
    explicit PartListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hasSelection() const;
    bool isRemovingAvailable() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void createNewPart();
    Q_INVOKABLE void removeSelectedParts();
    Q_INVOKABLE void openSelectedParts();

    Q_INVOKABLE void selectPart(int partIndex);
    Q_INVOKABLE void removePart(int partIndex);
    Q_INVOKABLE void setPartTitle(int partIndex, const QString& title);
    Q_INVOKABLE void validatePartTitle(int partIndex);
    Q_INVOKABLE void copyPart(int partIndex);

signals:
    void selectionChanged();
    void partAdded(int index);

private:
    void setTitle(INotationPtr notation, const QString& title);

    bool isExcerptIndexValid(int index) const;

    bool userAgreesToRemoveParts(int partCount) const;
    void doRemovePart(int partIndex);

    IMasterNotationPtr masterNotation() const;

    void insertExcerpt(int destinationIndex, IExcerptNotationPtr excerpt);
    void notifyAboutNotationChanged(int index);

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsCreated
    };

    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
    QList<IExcerptNotationPtr> m_excerpts;
    INotationPtr m_currentNotation;
};
}

#endif // MU_NOTATION_PARTLISTMODEL_H
