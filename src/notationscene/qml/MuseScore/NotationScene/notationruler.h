/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "ui/iuiconfiguration.h"

namespace mu::notation {
class NotationRuler : public muse::Injectable
{
    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };

public:
    NotationRuler(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void paint(muse::draw::Painter* painter, const NoteInputState& state);

private:
    enum LineType {
        CurrentPosition,
        MainBeat,
        Subdivision,
    };

    static LineType lineType(int lineTicks, int inputTicks, size_t lineIdx);

    void paintLine(muse::draw::Painter* painter, LineType type, const muse::PointF& point, double spatium, voice_idx_t voiceIdx);
};
}
