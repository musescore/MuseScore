/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engravingitem.h"

#include "draw/types/font.h"

namespace mu::engraving {
class SoundFlag final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, SoundFlag)
    DECLARE_CLASSOF(ElementType::SOUND_FLAG)

public:
    explicit SoundFlag(EngravingItem* parent = nullptr);

    SoundFlag* clone() const override;
    bool isEditable() const override;

    void setSelected(bool f) override;

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& value) override;
    PropertyValue propertyDefault(Pid id) const override;

    bool canBeExcludedFromOtherParts() const override { return false; }

    using PresetCodes = StringList;
    using PlayingTechniqueCode = String;

    const PresetCodes& soundPresets() const;
    void setSoundPresets(const PresetCodes& soundPresets);

    const PlayingTechniqueCode& playingTechnique() const;
    void setPlayingTechnique(const PlayingTechniqueCode& technique);

    bool play() const;
    void setPlay(bool play);

    bool applyToAllStaves() const;
    void setApplyToAllStaves(bool apply);

    void clear();

    bool shouldHide() const;

    void undoChangeSoundFlag(const PresetCodes& presets, const PlayingTechniqueCode& technique);

    char16_t iconCode() const;
    muse::draw::Font iconFont() const;
    void setIconFontSize(double size);
    Color iconBackgroundColor() const;

private:
    PresetCodes m_soundPresets;
    PlayingTechniqueCode m_playingTechnique;

    muse::draw::Font m_iconFont;
    bool m_iconFontValid = false;

    bool m_play = true;
    bool m_applyToAllStaves = true;
};
}

#endif // MU_ENGRAVING_SOUNDFLAG_H
