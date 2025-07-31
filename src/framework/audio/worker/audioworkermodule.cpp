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

#include "audioworkermodule.h"

#include "internal/workerplayback.h"
#include "internal/workerchannelcontroller.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio::worker;

std::string AudioWorkerModule::moduleName() const
{
    return "audio_worker";
}

void AudioWorkerModule::registerExports()
{
    m_workerPlayback = std::make_shared<WorkerPlayback>(iocContext());
    m_workerChannelController  = std::make_shared<WorkerChannelController>();

    ioc()->registerExport<IWorkerPlayback>(moduleName(), m_workerPlayback);
}

void AudioWorkerModule::onInit(const IApplication::RunMode&)
{
    m_workerPlayback->init();
    m_workerChannelController->init(m_workerPlayback);
}

void AudioWorkerModule::onDestroy()
{
    m_workerChannelController->deinit();
    m_workerPlayback->deinit();
}
