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

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class Score;
class EngravingItem;
class EditData;

class BarLine;
class Capo;
class Dynamic;
class Expression;
class FiguredBass;
class Fingering;
class GuitarBendText;
class Harmony;
class HarpPedalDiagram;
class InstrumentChange;
class InstrumentName;
class Jump;
class Lyrics;
class MMRestRange;
class Marker;
class MeasureNumber;
class PlayTechAnnotation;
class RehearsalMark;
class SlurTieSegment;
class StaffText;
class Sticking;
class StringTunings;
class SystemText;
class TempoText;
class Text;
class TextBase;
class TripletFeel;
}

namespace mu::engraving::rendering::editmode {
class EditModeDraw
{
public:

    static void drawItem(const EngravingItem* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);      // factory

private:
    static void drawEngravingItem(const EngravingItem* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);

    static void drawBarline(const BarLine* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);
    static void drawDynamic(const Dynamic* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);
    static void drawSlurTieSegment(const SlurTieSegment* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);
    static void drawTextBase(const TextBase* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling);
};
}
