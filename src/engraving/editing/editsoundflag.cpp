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

#include "editsoundflag.h"

#include "../dom/soundflag.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeSoundFlag
//---------------------------------------------------------

void ChangeSoundFlag::flip(EditData*)
{
    IF_ASSERT_FAILED(m_soundFlag) {
        return;
    }

    SoundFlag::PresetCodes presets = m_soundFlag->soundPresets();
    SoundFlag::PlayingTechniqueCode technique = m_soundFlag->playingTechnique();

    m_soundFlag->setSoundPresets(m_presets);
    m_soundFlag->setPlayingTechnique(m_playingTechnique);

    m_presets = std::move(presets);
    m_playingTechnique = std::move(technique);
}
