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
#ifndef MU_NOTATION_AUDIOOUTPUTDEVICECONTROLLER_H
#define MU_NOTATION_AUDIOOUTPUTDEVICECONTROLLER_H

#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"

#include "../iaudioconfiguration.h"
#include "../iaudiodriver.h"
#include "worker/iaudioengine.h"

namespace muse::audio {
class AudioOutputDeviceController : public Injectable, public async::Asyncable
{
    Inject<IAudioConfiguration> configuration = { this };
    Inject<IAudioDriver> audioDriver = { this };
    Inject<IAudioEngine> audioEngine = { this };

public:

    AudioOutputDeviceController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

private:
    void checkConnection();
    void onOutputDeviceChanged();
};
}

#endif // MU_NOTATION_AUDIOOUTPUTDEVICECONTROLLER_H
