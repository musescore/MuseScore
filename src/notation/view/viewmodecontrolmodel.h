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

#include <QObject>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "notation/notationtypes.h"

#include "ui/view/abstractmenumodel.h"

namespace mu::notation {
class ViewModeControlModel : public QObject, public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, context)

    Q_PROPERTY(QVariant currentViewMode READ currentViewMode NOTIFY currentViewModeChanged)
    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit ViewModeControlModel(QObject* parent = nullptr);

    QVariant currentViewMode();

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectViewMode(const QString& actionCode);

signals:
    void currentViewModeChanged();
    void itemsChanged();

private:
    void updateState();
    actions::ActionCode viewModeActionCode(ViewMode viewMode) const;

    ui::MenuItem m_currentViewMode;
};
}

#endif // MU_NOTATION_VIEWMODECONTROLMODEL_H
