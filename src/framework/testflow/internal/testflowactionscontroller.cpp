/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "testflowactionscontroller.h"

#include "types/uri.h"

using namespace muse::testflow;

static const muse::UriQuery SHOW_SCRIPTS_URI("muse://diagnostics/testflow/scripts?modal=false&floating=true");

void TestflowActionsController::init()
{
    dispatcher()->reg(this, "testflow-show-scripts", [this]() { openUri(SHOW_SCRIPTS_URI); });
}

void TestflowActionsController::openUri(const UriQuery& uri, bool isSingle)
{
    if (isSingle && interactive()->isOpened(uri.uri()).val) {
        return;
    }

    interactive()->open(uri);
}
