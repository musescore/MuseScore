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

#ifndef MU_NOTATION_NOTATIONTOOLBARMODEL_H
#define MU_NOTATION_NOTATIONTOOLBARMODEL_H

#include <QAbstractListModel>

#include "context/iglobalcontext.h"
#include "iinteractive.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "framework/ui/view/iconcodes.h"

namespace mu::notation {
class NotationToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, framework::IInteractive, interactive)

public:
    explicit NotationToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void open(int index);

private:
    bool hasNotation() const;

    enum Roles {
        TitleRole = Qt::UserRole + 1,
        IconRole,
        EnabledRole
    };

    struct ToolbarItem {
        QString title;
        int icon = 0;
        bool enabled = false;
        std::string uri;
    };

    ToolbarItem makeItem(std::string_view title, framework::IconCode::Code, std::string uri, bool enabled = true) const;

    QList<ToolbarItem> m_items;
};
}

#endif // MU_NOTATION_NOTATIONTOOLBARMODEL_H
