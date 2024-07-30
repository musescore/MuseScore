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

#ifndef MUSE_UPDATE_UPDATEMODEL_H
#define MUSE_UPDATE_UPDATEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iappupdateservice.h"

#include "async/asyncable.h"

namespace muse::update {
class UpdateModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int currentProgress READ currentProgress NOTIFY currentProgressChanged)
    Q_PROPERTY(int totalProgress READ totalProgress NOTIFY totalProgressChanged)
    Q_PROPERTY(QString progressTitle READ progressTitle NOTIFY progressTitleChanged)

    Inject<IAppUpdateService> service = { this };

public:
    explicit UpdateModel(QObject* parent = nullptr);
    ~UpdateModel();

    Q_INVOKABLE void load(const QString& mode);

    int currentProgress() const;
    int totalProgress() const;
    QString progressTitle() const;

signals:
    void started();
    void finished(int errorCode, const QString& installerPath);

    void currentProgressChanged();
    void totalProgressChanged();
    void progressTitleChanged();

private:
    void setCurrentProgress(int progress);
    void setTotalProgress(int progress);
    void setProgressTitle(const QString& title);

    int m_currentProgress = 0;
    int m_totalProgress = INT_MAX;
    QString m_progressTitle;
};
}

#endif // MUSE_UPDATE_UPDATEMODEL_H
