/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_PALETTE_PALETTELAYOUT_H
#define MU_PALETTE_PALETTELAYOUT_H

#include <memory>

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class EngravingItem;
class Score;

class Accidental;
class ActionIcon;
class Ambitus;
class Articulation;

class BagpipeEmbellishment;
class BarLine;
class Bracket;

class Clef;

class Fingering;
class FretDiagram;

class HarpPedalDiagram;

class KeySig;

class PlayTechAnnotation;

class Symbol;

class TempoText;
class TextBase;
class TimeSig;

class Volta;
}

namespace mu::palette {
class PaletteLayout
{
public:
    PaletteLayout() = default;

    static void layoutItem(engraving::EngravingItem* item);

    struct Context {
        Context(engraving::Score* s)
            : m_score(s) {}

        const engraving::MStyle& style() const;
        std::shared_ptr<engraving::IEngravingFont> engravingFont() const;

    private:
        engraving::Score* m_score = nullptr;
    };

    static void layout(engraving::Accidental* item, const Context& ctx);
    static void layout(engraving::ActionIcon* item, const Context& ctx);
    static void layout(engraving::Ambitus* item, const Context& ctx);
    static void layout(engraving::Articulation* item, const Context& ctx);

    static void layout(engraving::BagpipeEmbellishment* item, const Context& ctx);
    static void layout(engraving::BarLine* item, const Context& ctx);
    static void layout(engraving::Bracket* item, const Context& ctx);

    static void layout(engraving::Clef* item, const Context& ctx);

    static void layout(engraving::HarpPedalDiagram* item, const Context& ctx);

    static void layout(engraving::Fingering* item, const Context& ctx);
    static void layout(engraving::FretDiagram* item, const Context& ctx);

    static void layout(engraving::KeySig* item, const Context& ctx);

    static void layout(engraving::PlayTechAnnotation* item, const Context& ctx);

    static void layout(engraving::Symbol* item, const Context& ctx);

    static void layout(engraving::TempoText* item, const Context& ctx);
    static void layout(engraving::TimeSig* item, const Context& ctx);

    static void layout(engraving::Volta* item, const Context& ctx);

private:
    static void layoutTextBase(engraving::TextBase* item, const Context& ctx);
    static void layout1TextBase(engraving::TextBase* item, const Context& ctx);
};
}

#endif // MU_PALETTE_PALETTELAYOUT_H
