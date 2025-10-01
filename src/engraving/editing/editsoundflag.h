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

#include "undo.h"

#include "../dom/soundflag.h"

namespace mu::engraving {
class ChangeSoundFlag : public UndoCommand
{
    SoundFlag* m_soundFlag = nullptr;
    SoundFlag::PresetCodes m_presets;
    SoundFlag::PlayingTechniqueCode m_playingTechnique;

public:
    ChangeSoundFlag(SoundFlag* soundFlag, const SoundFlag::PresetCodes& presets, const SoundFlag::PlayingTechniqueCode& technique)
        : m_soundFlag(soundFlag), m_presets(presets), m_playingTechnique(technique) {}

    void flip(EditData*) override;
    UNDO_NAME("ChangeSoundFlag")
    UNDO_CHANGED_OBJECTS({ m_soundFlag })
};
}
