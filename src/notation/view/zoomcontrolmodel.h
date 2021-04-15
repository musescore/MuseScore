/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

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
