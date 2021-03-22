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
#ifndef MU_APPSHELL_STARTUPMODEL_H
#define MU_APPSHELL_STARTUPMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "iappshellconfiguration.h"
#include "actions/iactionsdispatcher.h"

namespace mu::appshell {
class StartupModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)

public:
    explicit StartupModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

private:
    std::string startupPageUri() const;
};
}

#endif // MU_APPSHELL_STARTUPMODEL_H
