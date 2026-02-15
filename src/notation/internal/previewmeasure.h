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

#include "draw/types/geometry.h"
#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class PreviewMeasure
{
    muse::GlobalInject<INotationConfiguration> configuration;

public:
    PreviewMeasure() = default;

    void setScore(const engraving::Score* score);

    bool isValid() const;
    muse::RectF rect() const;

    void paint(muse::draw::Painter* painter);

private:
    void paintStaffLines(muse::draw::Painter* painter, const muse::PointF& pos, double width, int lines, double lineDist, double lineWidth);

    const engraving::Score* m_score = nullptr;
};
}
