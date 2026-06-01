/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#include "playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::playback;

static const std::vector<CommandInfo> s_commandInfos = {
    CommandInfo{
        Command("command://playback/play"),
        TranslatableString("playback", "Play"),
        TranslatableString("playback", "Play the current score"),
        InputSchema(),
        Decoration(IconCode::Code::PLAY)
    },
    CommandInfo{
        Command("command://playback/pause"),
        TranslatableString("playback", "Pause"),
        TranslatableString("playback", "Pause playback"),
        InputSchema(),
        Decoration(IconCode::Code::PAUSE)
    },
    CommandInfo{
        Command("command://playback/stop"),
        TranslatableString("playback", "Stop"),
        TranslatableString("playback", "Stop playback"),
        InputSchema(),
        Decoration(IconCode::Code::STOP)
    }
};

std::string PlaybackCommands::moduleName() const
{
    return "playback";
}

const std::vector<CommandInfo>& PlaybackCommands::commandInfos() const
{
    return s_commandInfos;
}
