/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
class Measure;

class ExchangeVoices
{
public:
    static void exchangeVoicesInSelection(Score* score, voice_idx_t srcVoice, voice_idx_t dstVoice);
    static void exchangeVoices(Score* score, Measure* measure, voice_idx_t srcVoice, voice_idx_t dstVoice, staff_idx_t srcStaff,
                               staff_idx_t dstStaff);
};
}
