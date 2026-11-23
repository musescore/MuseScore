/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "modularity/ioc.h"

#include "notation/inotationconfiguration.h"

namespace mu::notation {
// The number or ratio of a tuplet waiting for its first duration in note input. Like the ruler, it is drawn at a
// fixed distance from the staff at the input position, over whatever is there: above the staff, between it and the
// ruler, or below it in voices 2 and 4.
class PendingTupletMarker : public muse::Contextable
{
    muse::GlobalInject<INotationConfiguration> configuration;

public:
    PendingTupletMarker(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void paint(muse::draw::Painter* painter, const NoteInputState& state);
};
}
