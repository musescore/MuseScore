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
#pragma once

#include <qqmlintegration.h>

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "media/ivideoencoderresolver.h"

namespace mu::preferences {
class VideoPreferencesModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT;

    muse::GlobalInject<muse::media::IVideoEncoderResolver> videoEncoderResolver;

    Q_PROPERTY(int ffmpegVersion READ ffmpegVersion NOTIFY ffmpegVersionChanged FINAL)
    Q_PROPERTY(QString ffmpegDir READ ffmpegDir WRITE setFFmpegDir NOTIFY ffmpegDirChanged FINAL)

public:
    explicit VideoPreferencesModel(QObject* parent = nullptr);
    ~VideoPreferencesModel();

    Q_INVOKABLE void load();

    int ffmpegVersion() const;

    QString ffmpegDir() const;
    void setFFmpegDir(const QString& dir);

signals:
    void ffmpegVersionChanged();
    void ffmpegDirChanged();

private:
    void enableVideoExportSettingMode();
    void disableVideoExportSettingMode();
};
}
