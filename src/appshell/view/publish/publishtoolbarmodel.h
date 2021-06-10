/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_APPSHELL_PUBLISHTOOLBARMODEL_H
#define MU_APPSHELL_PUBLISHTOOLBARMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"

namespace mu::appshell {
class PublishToolBarModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, ui::IUiActionsRegister, actionsRegister)

    Q_PROPERTY(bool printScoreEnabled READ printScoreEnabled NOTIFY printScoreEnabledChanged)
    Q_PROPERTY(bool uploadScoreEnabled READ uploadScoreEnabled NOTIFY uploadScoreEnabledChanged)
    Q_PROPERTY(bool exportScoreEnabled READ exportScoreEnabled NOTIFY exportScoreEnabledChanged)
    Q_PROPERTY(bool imageCaptureEnabled READ imageCaptureEnabled NOTIFY imageCaptureEnabledChanged)

public:
    PublishToolBarModel(QObject* parent = nullptr);

    bool printScoreEnabled() const;
    bool uploadScoreEnabled() const;
    bool exportScoreEnabled() const;
    bool imageCaptureEnabled() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void printScore();
    Q_INVOKABLE void uploadScore();
    Q_INVOKABLE void exportScore();
    Q_INVOKABLE void startImageCapture();

signals:
    void printScoreEnabledChanged();
    void uploadScoreEnabledChanged();
    void exportScoreEnabledChanged();
    void imageCaptureEnabledChanged();
};
}

#endif // MU_APPSHELL_PUBLISHTOOLBARMODEL_H
