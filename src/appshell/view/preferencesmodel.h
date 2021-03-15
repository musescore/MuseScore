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

#include <QObject>

#include "actions/iactionsdispatcher.h"
#include "actions/iactionsregister.h"
#include "modularity/ioc.h"

namespace mu::appshell {
class PreferencesModel : public QObject
{
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, actions::IActionsRegister, actionsRegister)

public:
    explicit PreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void resetFactorySettings();
    Q_INVOKABLE bool apply();
};
}

#endif // MU_APPSHELL_PREFERENCESMODEL_H
