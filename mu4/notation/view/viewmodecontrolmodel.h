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

#ifndef MU_NOTATION_VIEWMODECONTROLMODEL_H
#define MU_NOTATION_VIEWMODECONTROLMODEL_H

#include <QtQml>
#include <QList>
#include <QHash>
#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "inotationconfiguration.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "notation/notationtypes.h"

namespace mu {
namespace notation {
class ViewModeControlModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, actions::IActionsDispatcher, dispatcher);
    INJECT(notation, context::IGlobalContext, globalContext);

    Q_PROPERTY(int currentViewModeId READ currentViewModeId WRITE setCurrentViewModeId NOTIFY currentViewModeIdChanged)

public:
    enum NoleNames {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit ViewModeControlModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentViewModeId() const;

    Q_INVOKABLE void load();

    void updateState();

signals:
    void currentViewModeIdChanged(int viewModeId);

public slots:
    void setCurrentViewModeId(int newViewModeId);

private:

    int getIdFromViewMode(const ViewMode& viewMode);
    void onNotationChanged();

    int m_currentViewModeId = 0;  // default to page view

    QList<ViewModeOption> m_viewModeOptions;
    QHash<int, QByteArray> m_roleNames;

    async::Notification m_notationChanged;
};
}
}

#endif // MU_NOTATION_VIEWMODECONTROLMODEL_H
