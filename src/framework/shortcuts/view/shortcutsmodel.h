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

#ifndef MU_SHORTCUTS_SHORTCUTSMODEL_H
#define MU_SHORTCUTS_SHORTCUTSMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "ishortcutsregister.h"
#include "actions/iactionsregister.h"

namespace mu::shortcuts {
class ShortcutsModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(shortcuts, IShortcutsRegister, shortcutsRegister)
    INJECT(shortcuts, actions::IActionsRegister, actionsRegister)

public:
    explicit ShortcutsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

private:
    QString actionTitle(const std::string& actionCode) const;

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleSequence,
        RoleSearchKey
    };

    QList<Shortcut> m_shortcuts;
};
}

#endif // MU_SHORTCUTS_SHORTCUTSMODEL_H
