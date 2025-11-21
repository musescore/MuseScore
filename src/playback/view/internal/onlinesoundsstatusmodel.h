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
#include "audio/main/iaudioconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "tours/itoursservice.h"

namespace mu::playback {
class OnlineSoundsStatusModel : public QObject, public muse::async::Asyncable, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<IPlaybackController> playbackController = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::audio::IAudioConfiguration> audioConfiguration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::tours::IToursService> tours = { this };

    Q_PROPERTY(bool hasOnlineSounds READ hasOnlineSounds NOTIFY hasOnlineSoundsChanged)
    Q_PROPERTY(bool manualProcessingAllowed READ manualProcessingAllowed NOTIFY manualProcessingAllowedChanged)

    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorTitle READ errorTitle NOTIFY statusChanged)
    Q_PROPERTY(QString errorDescription READ errorDescription NOTIFY statusChanged)

public:
    enum class Status {
        Processing,
        Success,
        Error,
    };

    Q_ENUM(Status);

    explicit OnlineSoundsStatusModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void processOnlineSounds();

    bool hasOnlineSounds() const;
    bool manualProcessingAllowed() const;

    int status() const;
    QString errorTitle() const;
    QString errorDescription() const;

signals:
    void hasOnlineSoundsChanged();
    void manualProcessingAllowedChanged();
    void statusChanged();

private:
    void onOnlineSoundsChanged();
    void updateManualProcessingAllowed(bool enableByDefault = true);

    void setManualProcessingAllowed(bool allowed);
    void setStatus(Status status);

    notation::InstrumentTrackIdSet m_onlineTrackIdSet;
    bool m_manualProcessingAllowed = false;
    bool m_shouldNotifyToursThatManualProcessingAllowed = true;
    Status m_status = Status::Success;
    muse::Ret m_ret;
    muse::async::Channel<notation::InstrumentTrackIdSet> m_tracksDataChanged;
};
}
