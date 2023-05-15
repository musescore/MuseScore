/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "scorethumbnailloader.h"

#include <QtConcurrent>

using namespace mu::project;

ScoreThumbnailLoader::ScoreThumbnailLoader(QObject* parent)
    : QObject(parent)
{
    m_finishedLoading.onReceive(this, [this](const RetVal<ProjectMeta>& rv) {
        if (!rv.ret) {
            setThumbnail(QPixmap());
        } else {
            setThumbnail(rv.val.thumbnail);
        }
    });
}

QString ScoreThumbnailLoader::scorePath() const
{
    return m_scorePath;
}

void ScoreThumbnailLoader::setScorePath(const QString& scorePath)
{
    if (m_scorePath == scorePath) {
        return;
    }

    m_scorePath = scorePath;
    emit scorePathChanged();

    loadThumbnail();
}

bool ScoreThumbnailLoader::isThumbnailValid() const
{
    return !m_thumbnail.isNull();
}

QPixmap ScoreThumbnailLoader::thumbnail() const
{
    return m_thumbnail;
}

void ScoreThumbnailLoader::loadThumbnail()
{
    QtConcurrent::run([finishedLoading = m_finishedLoading, scorePath = m_scorePath]() mutable {
        finishedLoading.send(mscMetaReader()->readMeta(scorePath));
    });
}

void ScoreThumbnailLoader::setThumbnail(const QPixmap& thumbnail)
{
    m_thumbnail = thumbnail;
    emit thumbnailChanged();
}
