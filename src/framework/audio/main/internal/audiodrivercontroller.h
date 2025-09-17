/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "audio/iaudiodrivercontroller.h"
#include "modularity/ioc.h"

#include "audio/main/iaudioconfiguration.h"

namespace muse::audio {
class AudioDriverController : public IAudioDriverController, public Injectable
{
    Inject<IAudioConfiguration> configuration = { this };

public:
    AudioDriverController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;
    async::Notification currentAudioApiChanged() const override;

    std::vector<std::string> availableAudioApiList() const override;

    IAudioDriverPtr audioDriver() const override;

private:
    IAudioDriverPtr m_audioDriver;
};
}
