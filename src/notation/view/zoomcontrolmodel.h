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

#ifndef MU_NOTATION_ZOOMCONTROLMODEL_H
#define MU_NOTATION_ZOOMCONTROLMODEL_H

#include <QtQml>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "inotationconfiguration.h"
#include "async/asyncable.h"

namespace mu {
namespace notation {
class ZoomControlModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, INotationConfiguration, configuration)

    Q_PROPERTY(int currentZoom READ currentZoom NOTIFY currentZoomChanged)

public:
    int currentZoom() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

signals:
    void currentZoomChanged(int zoom);

private:
    void setCurrentZoom(int zoom);

    int m_currentZoom = 0;
};
}
}

#endif // MU_NOTATION_ZOOMCONTROLMODEL_H
