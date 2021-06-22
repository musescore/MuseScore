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
#include "diagnosticsactionscontroller.h"

#include "uri.h"

using namespace mu::diagnostics;

static const mu::UriQuery DIAGNOSTICS_URI("musescore://diagnostics/main?sync=false&modal=false");

void DiagnosticsActionsController::init()
{
    dispatcher()->reg(this, "diagnostics-show", [this]() {
        if (!interactive()->isOpened(DIAGNOSTICS_URI.uri()).val) {
            interactive()->open(DIAGNOSTICS_URI);
        }
    });
}
