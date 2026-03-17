/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "videopreferencesmodel.h"

using namespace mu::preferences;

VideoPreferencesModel::VideoPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void VideoPreferencesModel::load()
{
    videoEncoderResolver()->loadedFFmpegChanged().onNotify(this, [this]() {
        emit ffmpegVersionChanged();
        emit ffmpegDirChanged();
    });

    emit ffmpegVersionChanged();
    emit ffmpegDirChanged();
}

int VideoPreferencesModel::ffmpegVersion() const
{
    return videoEncoderResolver()->loadedFFmpegVersion();
}

QString VideoPreferencesModel::ffmpegDir() const
{
    return videoEncoderResolver()->loadedFFmpegDir().toQString();
}

void VideoPreferencesModel::setFFmpegDir(const QString& dir)
{
    if (ffmpegDir() == dir) {
        return;
    }

    videoEncoderResolver()->loadFFmpeg(dir);
}
