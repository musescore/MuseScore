//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_NOTATION_PARTLISTMODEL_H
#define MU_NOTATION_PARTLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "inotationcreator.h"

class QItemSelectionModel;

namespace mu::notation {
class PartListModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, INotationCreator, notationCreator)

public:
    explicit PartListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void createNewPart();
    Q_INVOKABLE void removeSelectedParts();
    Q_INVOKABLE void openSelectedParts();
    Q_INVOKABLE void apply();

    Q_INVOKABLE void selectPart(int index);
    Q_INVOKABLE void removePart(int index);
    Q_INVOKABLE void setPartTitle(int index, const QString& title);
    Q_INVOKABLE void setVoicesVisibility(int index, const QVariantList& visibility);
    Q_INVOKABLE void copyPart(int index);

private:
    QString formatVoicesTitle(INotationPtr notation) const;
    QVariantList voicesVisibility(INotationPtr notation) const;

    void setTitle(INotationPtr notation, const QString& title);

    bool isIndexValid(int index) const;
    IMasterNotationPtr masterNotation() const;
    QList<int> selectedRows() const;

    void insertNotation(int destinationIndex, INotationPtr notation);
    void notifyAboutNotationChanged(int index);

    void applyVoicesVisibility(INotationPtr notation) const;

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsMain,
        RoleVoicesVisibility,
        RoleVoicesTitle
    };

    QItemSelectionModel* m_selectionModel = nullptr;
    QHash<int, QByteArray> m_roles;
    QList<INotationPtr> m_notations;
    QHash<QString /* notation key */, QVariantList /* voices */> m_voicesVisibility;
    INotationPtr m_currentNotation;
};
}

#endif // MU_NOTATION_PARTLISTMODEL_H
