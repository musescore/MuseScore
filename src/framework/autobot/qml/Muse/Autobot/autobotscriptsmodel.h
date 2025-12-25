/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "iautobot.h"
#include "iautobotscriptsrepository.h"

namespace muse::autobot {
class AutobotScriptsModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool isRunAllTCMode READ isRunAllTCMode WRITE setIsRunAllTCMode NOTIFY isRunAllTCModeChanged)
    Q_PROPERTY(QString speedMode READ speedMode WRITE setSpeedMode NOTIFY speedModeChanged)

    QML_ELEMENT

    Inject<IAutobotScriptsRepository> scriptsRepository = { this };
    Inject<IAutobot> autobot = { this };

public:
    explicit AutobotScriptsModel(QObject* parent = nullptr);
    ~AutobotScriptsModel();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isRunAllTCMode() const;
    QString speedMode() const;
    void setSpeedMode(const QString& newSpeedMode);

    Q_INVOKABLE void load();
    Q_INVOKABLE void runScript(int scriptIndex);

    Q_INVOKABLE void runAllTC();
    Q_INVOKABLE bool tryRunNextTC();
    Q_INVOKABLE void stopRunAllTC();

    Q_INVOKABLE void toggleSelect(int index);
    Q_INVOKABLE void toggleAllSelect(const QString& type);
    Q_INVOKABLE bool isAllSelected(const QString& type) const;

public slots:
    void setIsRunAllTCMode(bool arg);

signals:
    void isRunAllTCModeChanged();
    void requireStartTC(const QString& path);
    void isAllSelectedChanged(const QString& type, bool arg);
    void speedModeChanged();

private:

    enum Roles {
        rTitle = Qt::UserRole + 1,
        rDescription,
        rType,
        rPath,
        rStatus,
        rSelected
    };

    void setStatus(const io::path_t& path, IAutobot::Status st);
    bool isAllSelected(const ScriptType& type) const;

    Scripts m_scripts;
    int m_currentTCIndex = -1;
    bool m_isRunAllTCMode = false;
    QMap<io::path_t, IAutobot::Status> m_statuses;
    QMap<int, bool> m_selected;
};
}
