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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <functional>

#include "draw/types/geometry.h"
#include "types/mimedata.h"

namespace muse::draw {
class Painter;
}

namespace mu::notation {
//! NOTE The data of an element or a range that is dragged out of the score.
//! The view takes it and does the platform part of the dragging.
struct OutgoingDrag {
    muse::MimeData mimeData;
    bool isMoveAllowed = false;

    //! The preview of the dragged data, in the logical (score) coordinates. If the rect is empty, there is no preview
    muse::RectF previewRect;
    std::function<void(muse::draw::Painter*)> paintPreview;
};
}
