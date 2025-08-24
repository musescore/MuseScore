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

#include <array>
#include <map>
#include <vector>

#include "engraving/dom/mscore.h"
#include "engraving/types/types.h"

namespace mu::iex::tabledit {
struct TefNote;

class VoiceAllocator
{
public:
    void addColumn(const std::vector<const TefNote*>& column);
    void addNote(const TefNote* const note, const bool preferVoice0);
    void allocateVoice(const TefNote* const note, int voice);
    bool canAddTefNoteToVoice(const TefNote* const note, const int voice);
    void dump();
    int findFirstPossibleVoice(const TefNote* const note, const std::array<int, 3> voices);
    int stopPosition(const size_t voice);
    int voice(const TefNote* const note);
    const std::vector<std::vector<const TefNote*> >& voiceContent(mu::engraving::voice_idx_t voice) const
    {
        return voiceContents.at(voice);
    }

private:
    void appendNoteToVoice(const TefNote* const note, int voice);
    std::map<const TefNote*, int> allocations;
    std::array<const TefNote*, mu::engraving::VOICES> notesPlaying = { nullptr, nullptr, nullptr, nullptr };
    std::array<std::vector<std::vector<const TefNote*> >, mu::engraving::VOICES> voiceContents;
};
} // namespace mu::iex::tabledit
