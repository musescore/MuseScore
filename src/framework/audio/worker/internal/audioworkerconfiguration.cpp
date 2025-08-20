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
#include "audioworkerconfiguration.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::worker;
using namespace muse::audio::rpc;

void AudioWorkerConfiguration::init()
{
    channel()->onMethod(Method::WorkerConfigChanged, [this](const Msg& msg) {
        LOGI() << "WorkerConfigChanged";

        WorkerConf conf;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, conf)) {
            return;
        }

        //! TODO Check changes?
        m_conf = conf;

        m_inited = true;
        channel()->send(rpc::make_notification(Method::WorkerConfigInited));
    });
}

audioch_t AudioWorkerConfiguration::audioChannelsCount() const
{
}

sample_rate_t AudioWorkerConfiguration::sampleRate() const
{
    return m_conf.sampleRate;
}
