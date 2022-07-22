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
#include "autobotactionscontroller.h"

#include "types/uri.h"

using namespace mu::autobot;

static const mu::UriQuery SHOW_BATCHTESTS_URI("musescore://autobot/batchtests?sync=false&modal=false&floating=true");
static const mu::UriQuery SHOW_SCRIPTS_URI("musescore://autobot/scripts?sync=false&modal=false&floating=true");

void AutobotActionsController::init()
{
    dispatcher()->reg(this, "autobot-show-batchtests", [this]() { openUri(SHOW_BATCHTESTS_URI); });
    dispatcher()->reg(this, "autobot-show-scripts", [this]() { openUri(SHOW_SCRIPTS_URI); });
}

void AutobotActionsController::openUri(const mu::UriQuery& uri, bool isSingle)
{
    if (isSingle && interactive()->isOpened(uri.uri()).val) {
        return;
    }

    interactive()->open(uri);
}
