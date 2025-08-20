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

#include "../iaudioworkerconfiguration.h"

#include "global/modularity/ioc.h"
#include "audio/common/rpc/irpcchannel.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::worker {
class AudioWorkerConfiguration : public IAudioWorkerConfiguration
{
    Inject<rpc::IRpcChannel> channel;

public:
    AudioWorkerConfiguration() = default;

    void init();

    audioch_t audioChannelsCount() const override;
    sample_rate_t sampleRate() const override;

private:

    bool m_inited = false;
    WorkerConf m_conf;
};
}
