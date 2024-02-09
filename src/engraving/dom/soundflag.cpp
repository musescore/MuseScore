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

#include "soundflag.h"

#include "draw/fontmetrics.h"

#include "undo.h"

using namespace mu::engraving;

SoundFlag::SoundFlag(EngravingItem* parent)
    : EngravingItem(ElementType::SOUND_FLAG, parent)
{
}

bool SoundFlag::isEditable() const
{
    return false;
}

SoundFlag* SoundFlag::clone() const
{
    return new SoundFlag(*this);
}

const SoundFlag::PresetCodes& SoundFlag::soundPresets() const
{
    return m_soundPresets;
}

void SoundFlag::setSoundPresets(const PresetCodes& soundPresets)
{
    m_soundPresets = soundPresets;
}

const SoundFlag::Params& SoundFlag::params() const
{
    return m_params;
}

void SoundFlag::setParams(const Params& params)
{
    m_params = params;
}

bool SoundFlag::isTextVisible() const
{
    return m_isTextVisible;
}

void SoundFlag::setIsTextVisible(bool visible)
{
    m_isTextVisible = visible;
}

void SoundFlag::undoChangeSoundFlag(const PresetCodes& presets, const Params& params, bool isTextVisible)
{
    score()->undo(new ChangeSoundFlag(this, presets, params, isTextVisible));
    triggerLayout();
}

char16_t SoundFlag::iconCode() const
{
    return 0xEF4E;
}

draw::Font SoundFlag::iconFont() const
{
    return m_iconFont;
}

Color SoundFlag::iconBackgroundColor() const
{
    Color color = curColor();
    if (!selected()) {
        color = Color("#CFD5DD");
        color.setAlpha(128);
    }

    return color;
}

RectF SoundFlag::canvasBoundingIconRect() const
{
    return ldata()->iconBBox.translated(canvasPos());
}
