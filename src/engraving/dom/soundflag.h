/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_SOUNDFLAG_H
#define MU_ENGRAVING_SOUNDFLAG_H

#include "textbase.h"

#include "global/types/val.h"

namespace mu::engraving {
class SoundFlag final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, SoundFlag)
    DECLARE_CLASSOF(ElementType::SOUND_FLAG)

public:
    SoundFlag(Segment* parent = nullptr);

    SoundFlag* clone() const override;

    using PresetCodes = StringList;
    using Params = std::map<String, Val>;

    const PresetCodes& soundPresets() const;
    void setSoundPresets(const PresetCodes& soundPresets);

    const Params& params() const;
    void setParams(const Params& params);

    void undoChangeSoundFlag(const PresetCodes& presets, const Params& params);

private:
    PresetCodes m_soundPresets;
    Params m_params;
};
}

#endif // MU_ENGRAVING_SOUNDFLAG_H
