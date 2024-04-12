/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PLAYBACK_PLAYBACKLOADINGMODEL_H
#define MU_PLAYBACK_PLAYBACKLOADINGMODEL_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iplaybackcontroller.h"

namespace mu::playback {
class PlaybackLoadingModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(IPlaybackController, playbackController)

    Q_PROPERTY(int currentProgress READ currentProgress NOTIFY currentProgressChanged)
    Q_PROPERTY(int totalProgress READ totalProgress NOTIFY totalProgressChanged)
    Q_PROPERTY(QString progressTitle READ progressTitle NOTIFY progressTitleChanged)

public:
    explicit PlaybackLoadingModel(QObject* parent = nullptr);

    int currentProgress() const;
    int totalProgress() const;
    QString progressTitle() const;

    Q_INVOKABLE void load();

signals:
    void currentProgressChanged();
    void totalProgressChanged();
    void progressTitleChanged();

    void started();
    void finished();

private:
    void setCurrentProgress(int progress);
    void setTotalProgress(int progress);
    void setProgressTitle(const QString& title);

    int m_currentProgress = 0;
    int m_totalProgress = INT_MAX;
    QString m_progressTitle;
};
}

#endif // MU_PLAYBACK_PLAYBACKLOADINGMODEL_H
