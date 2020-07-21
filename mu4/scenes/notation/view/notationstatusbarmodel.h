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

#ifndef MU_NOTATIONSCENE_NOTATIONSTATUSBARMODEL_H
#define MU_NOTATIONSCENE_NOTATIONSTATUSBARMODEL_H

#include <QtQml>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "iscenenotationconfiguration.h"

namespace mu {
namespace scene {
namespace notation {
class NotationStatusBarModel : public QObject, public mu::async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, ISceneNotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(QString accessibilityInfo READ accessibilityInfo NOTIFY accessibilityInfoChanged)
    Q_PROPERTY(int currentZoom READ currentZoom NOTIFY currentZoomChanged)

public:
    QString accessibilityInfo() const;
    int currentZoom() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

signals:
    void accessibilityInfoChanged(const QString &info);
    void currentZoomChanged(int zoom);

private: 
    void setCurrentZoom(int zoom);

    QString m_accessibilityInfo;
    int m_currentZoom = 0;
};
}
}
}

#endif // MU_NOTATIONSCENE_NOTATIONSTATUSBARMODEL_H
