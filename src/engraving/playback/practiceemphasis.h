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

#include "mpe/events.h"

namespace mu::engraving {
class Note;
class Score;
struct NominalNoteCtx;

//! Practice-track emphasis. Playback and audio export share this path.
//! The score's practice-emphasis.json sidecar is the only control: emphasized
//! notes are scaled onto a companion staff layer (staff + EMPHASIS_STAFF_OFFSET)
//! and the rest are attenuated.
class PracticeEmphasis
{
public:
    static constexpr int EMPHASIS_STAFF_OFFSET = 256;
    static constexpr float DB_PER_DYNAMIC_STEP = 6.f;

    static muse::mpe::dynamic_level_t scaleDynamicLevel(muse::mpe::dynamic_level_t level, float gainDb);

    static void applyToNote(const Note* note, NominalNoteCtx& ctx);
    static void applyToDynamics(const Score* score, muse::mpe::DynamicAutomationLayers& layers);
};
}
