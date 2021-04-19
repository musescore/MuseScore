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
#include "midiinputcontroller.h"
#include "log.h"

using namespace mu::notation;

void MidiInputController::init()
{
    midiInPort()->eventReceived().onReceive(this, [this](const std::pair<midi::tick_t, midi::Event>& ev) {
        if (!configuration()->isMidiInputEnabled()) {
            return;
        }

        onMidiEventReceived(ev.first, ev.second);
    });
}

void MidiInputController::onMidiEventReceived(midi::tick_t tick, const midi::Event& event)
{
    UNUSED(tick);

    if (midiRemote()->isSettingMode()) {
        midiRemote()->setCurrentActionEvent(event);
        return;
    }

    Ret ret = midiRemote()->process(event);
    if (check_ret(ret, Ret::Code::Undefined)) {
        //! NOTE Event not command, pass further
        // pass
    } else if (!ret) {
        LOGE() << "failed midi remote process, err: " << ret.toString();
        // pass
    } else {
        //! NOTE Success midi remote process (the event is command)
        return;
    }

    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->midiInput()->onMidiEventReceived(event);
    }
}
