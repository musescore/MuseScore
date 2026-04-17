/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "transporteventscontroller.h"

#include "audio/common/audiosanitizer.h"

using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::actions;

void TransportEventsController::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onMethod(Method::TransportEventReceived, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;

        TransportEvent event;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, event)) {
            return;
        }

        onEventReceived(event);
    });
}

void TransportEventsController::deinit()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onMethod(Method::TransportEventReceived, nullptr);
}

void TransportEventsController::onEventReceived(const TransportEvent& event)
{
    ONLY_AUDIO_MAIN_THREAD;

    switch (event.type) {
    case TransportEvent::Type::Play:
        dispatcher()->dispatch("play");
        break;
    case TransportEvent::Type::Pause:
        dispatcher()->dispatch("pause");
        break;
    case TransportEvent::Type::Stop:
        dispatcher()->dispatch("stop");
        break;
    case TransportEvent::Type::Seek:
        if (std::holds_alternative<TransportEvent::SeekData>(event.data)) {
            const secs_t pos = std::get<TransportEvent::SeekData>(event.data).position;
            dispatcher()->dispatch("rewind", ActionData::make_arg1<secs_t>(pos));
        }
        break;
    case TransportEvent::Type::Unknown:
        break;
    }
}
