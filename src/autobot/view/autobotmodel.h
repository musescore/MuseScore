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
#ifndef MU_AUTOBOT_AUTOBOTMODEL_H
#define MU_AUTOBOT_AUTOBOTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iautobot.h"

namespace mu::autobot {
class AutobotModel : public QObject
{
    INJECT(autobot, IAutobot, autobot)

    Q_OBJECT
public:
    explicit AutobotModel(QObject* parent = nullptr);

    Q_INVOKABLE void run();
    Q_INVOKABLE void stop();
};
}

#endif // MU_AUTOBOT_AUTOBOTMODEL_H
