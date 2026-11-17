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

#include "pendingtupletmarker.h"

#include "draw/fontmetrics.h"
#include "draw/painter.h"
#include "draw/types/font.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tuplet.h"
#include "engraving/editing/noteinput.h"
#include "engraving/iengravingfont.h"
#include "engraving/style/defaultstyle.h"
#include "engraving/types/symnames.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

// in staff spaces: the distance of the number's centre from the staff, halfway to the ruler
static constexpr double DISTANCE_FROM_STAFF = 1.0;
// in staff spaces: the most the number may fill of the 2 spaces between the staff and the ruler
static constexpr double MAX_HEIGHT = 1.8;

// the music font's tuplet digits for "3" or "3:2"
static SymIdList tupletSymbols(const String& digits)
{
    SymIdList symbols;
    for (size_t i = 0; i < digits.size(); ++i) {
        const Char c = digits.at(i);
        symbols.push_back(c == u':' ? SymId::tupletColon : SymNames::symIdByName(u"tuplet" + String(c)));
    }
    return symbols;
}

void PendingTupletMarker::paint(Painter* painter, const NoteInputState& state)
{
    if (!state.isValid()) {
        return;
    }

    const Staff* staff = state.staff();
    const Segment* segment = state.segment();
    const Measure* measure = segment ? segment->measure() : nullptr;
    const System* system = measure ? measure->system() : nullptr;
    const SysStaff* sysStaff = system ? system->staff(state.staffIdx()) : nullptr;
    if (!staff || !sysStaff) {
        return;
    }

    const Score* score = staff->score();
    const std::optional<TupletPreview> tuplet = NoteInput::tupletPreview(score);
    if (!tuplet) {
        return;
    }

    const MStyle& style = score->style();
    const double spatium = style.spatium();
    // the default size of a tuplet number, whatever the score's style sets
    const MStyle& defaultStyle = DefaultStyle::defaultStyle();
    const double spatiumScaling = spatium / style.defaultSpatium();

    // at the input position, where the ruler and the cursor show it
    const PointF measurePos = measure->canvasPos();
    const double x = state.beyondScore() ? score->lastMeasure()->pageBoundingRect().right() + spatium
                     : measurePos.x() + segment->x();

    // above the staff, or below it in voices 2 and 4
    const double staffTop = measurePos.y() + sysStaff->y();
    const double staffBottom = staffTop + staff->staffHeight(segment->tick());
    const double centreY = state.voice() % 2 == 0 ? staffTop - DISTANCE_FROM_STAFF * spatium
                           : staffBottom + DISTANCE_FROM_STAFF * spatium;

    // the colour of the input preview notes
    Color color = configuration()->noteInputPreviewColor();
    if (!color.isValid() || color == configuration()->selectionColor()) {
        color = configuration()->highlightSelectionColor(state.voice());
    }
    painter->setPen(color);

    // the number or ratio as set for the tuplet; just the number if the score hides it
    const TupletNumberType numberType = tuplet->numberType == TupletNumberType::NO_TEXT
                                        ? TupletNumberType::SHOW_NUMBER : tuplet->numberType;
    const String digits = Tuplet::numberText(tuplet->ratio, numberType, /*useSymbols*/ false);

    // the score's tuplet text font
    Font font(style.styleSt(Sid::tupletFontFace), Font::Type::Text);
    const FontStyle fontStyle = style.styleV(Sid::tupletFontStyle).value<FontStyle>();
    font.setBold(fontStyle & FontStyle::Bold);
    font.setItalic(fontStyle & FontStyle::Italic);
    font.setPointSizeF(defaultStyle.styleD(Sid::tupletFontSize) * spatiumScaling);
    const double mag = defaultStyle.styleD(Sid::tupletMusicalSymbolsScale) * spatiumScaling;

    // or the music font's tuplet digits, as in the score
    const bool useSymbols = style.styleB(Sid::tupletUseSymbols);
    const SymIdList symbols = useSymbols ? tupletSymbols(digits) : SymIdList();
    const std::shared_ptr<IEngravingFont> musicFont = score->engravingFont();

    // shrunk if the font is oversized
    const RectF ink = useSymbols ? musicFont->bbox(symbols, mag) : FontMetrics(font).tightBoundingRect(digits);
    const double scale = std::min(1.0, MAX_HEIGHT * spatium / ink.height());
    const double baseline = centreY - ink.center().y() * scale;

    if (useSymbols) {
        musicFont->draw(symbols, painter, mag * scale, PointF(x - ink.left() * scale, baseline));
    } else {
        font.setPointSizeF(font.pointSizeF() * scale);
        painter->setFont(font);
        painter->drawText(PointF(x, baseline), digits);
    }
}
