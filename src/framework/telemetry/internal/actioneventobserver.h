//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#ifndef MU_TELEMETRY_ACTIONEVENTOBSERVER_H
#define MU_TELEMETRY_ACTIONEVENTOBSERVER_H

#include <QObject>
#include <QAction>
#include <QPair>

#include "modularity/ioc.h"
#include "itelemetryservice.h"

namespace mu::telemetry {
class ActionEventObserver : public QObject
{
    Q_OBJECT

    INJECT(telemetry, ITelemetryService, telemetryService)

public:
    static ActionEventObserver* instance();

    bool eventFilter(QObject* watched, QEvent* event) override;

//public slots:
//    void setScoreState(const Ms::ScoreState state);

private:
    Q_DISABLE_COPY(ActionEventObserver)

    explicit ActionEventObserver(QObject* parent = nullptr);
    QPair<QString, QString> extractActionData(QObject* watched);

//    Ms::ScoreState m_scoreState { Ms::STATE_INIT };
};
}

#endif // MU_TELEMETRY_ACTIONEVENTOBSERVER_H
