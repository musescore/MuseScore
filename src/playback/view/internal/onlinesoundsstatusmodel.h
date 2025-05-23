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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "playback/iplaybackcontroller.h"

namespace mu::playback {
class OnlineSoundsStatusModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(IPlaybackController, playbackController)

    Q_PROPERTY(bool hasOnlineSounds READ hasOnlineSounds NOTIFY hasOnlineSoundsChanged)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)

public:
    enum class Status {
        Processing,
        Success,
        Error,
    };

    Q_ENUM(Status);

    explicit OnlineSoundsStatusModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    bool hasOnlineSounds() const;
    int status() const;

signals:
    void hasOnlineSoundsChanged();
    void statusChanged();

private:
    void setHasOnlineSounds(bool value);
    void setStatus(Status status);

    bool m_hasOnlineSounds = false;
    Status m_status = Status::Success;
};
}
