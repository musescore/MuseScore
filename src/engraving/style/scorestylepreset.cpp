/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "scorestylepreset.h"

using namespace mu::engraving;

muse::io::path_t ScoreStylePresetHelper::getStyleFilePath(ScoreStylePreset preset) const
{
    muse::io::path_t basePath = globalConfiguration()->appDataPath() + "styles/MSN/";
    switch (preset) {
    case ScoreStylePreset::DEFAULT:
        return engravingConfiguration()->defaultStyleFilePath();
    case ScoreStylePreset::MSN_16MM:
        return basePath + "16mm_MSN.mss";
    case ScoreStylePreset::MSN_18MM:
        return basePath + "18mm_MSN.mss";
    case ScoreStylePreset::MSN_20MM:
        return basePath + "20mm_MSN.mss";
    case ScoreStylePreset::MSN_24MM:
        return basePath + "24mm_MSN.mss";
    case ScoreStylePreset::MSN_25MM:
        return basePath + "25mm_MSN.mss";
    default:
        return muse::io::path_t();
    }
}
