//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_PREFERENCESMODEL_H
#define MU_APPSHELL_PREFERENCESMODEL_H

#include <QAbstractItemModel>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/iactionsregister.h"
#include "ui/view/iconcodes.h"

#include "preferencepageitem.h"

namespace mu::appshell {
class PreferencesModel : public QAbstractItemModel
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, actions::IActionsRegister, actionsRegister)

    Q_PROPERTY(QString currentMenuId READ currentMenuId WRITE setCurrentMenuId NOTIFY currentMenuIdChanged)

public:
    explicit PreferencesModel(QObject* parent = nullptr);
    ~PreferencesModel();

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentMenuId() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void resetFactorySettings();
    Q_INVOKABLE bool apply();

public slots:
    void setCurrentMenuId(QString currentMenuId);

signals:
    void currentMenuIdChanged(QString currentMenuId);

private:

    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    PreferencePageItem* makeItem(const QString& id, const QString& title, ui::IconCode::Code icon = mu::ui::IconCode::Code::NONE,
                                 const QList<PreferencePageItem*>& children = {}) const;

    PreferencePageItem* modelIndexToItem(const QModelIndex& index) const;

    PreferencePageItem* m_rootItem = nullptr;
    QString m_currentMenuId;
};
}

#endif // MU_APPSHELL_PREFERENCESMODEL_H
