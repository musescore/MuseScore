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

#include "types/translatablestring.h"

namespace mu::notation {
enum class ZoomType : unsigned char {
    Percentage,
    PageWidth,
    WholePage,
    TwoPages
};

inline muse::TranslatableString zoomTypeTitle(ZoomType type)
{
    switch (type) {
    case ZoomType::Percentage: return muse::TranslatableString("notation", "Percentage");
    case ZoomType::PageWidth: return muse::TranslatableString("notation", "Page width");
    case ZoomType::WholePage: return muse::TranslatableString("notation", "Whole page");
    case ZoomType::TwoPages: return muse::TranslatableString("notation", "Two pages");
    }

    return {};
}
}
