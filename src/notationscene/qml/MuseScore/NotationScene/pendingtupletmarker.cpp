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
#include "draw/types/pen.h"

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
// in staff spaces: bracket line width, hook length, gap around the number, and gap before the next note
static constexpr double LINE_WIDTH = 0.1;
static constexpr double HOOK_LENGTH = 0.75;
static constexpr double NUMBER_GAP = 0.35;
static constexpr double END_GAP = 0.35;
// in staff spaces: the open-ended bracket past the end of the score
static constexpr double BEYOND_SCORE_LENGTH = 4.0;

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

// the x position of `tick` in `measure`, spaced evenly between chords and rests as the ruler does
static double measureXAt(const Measure* measure, const Fraction& tick)
{
    Fraction prevTick = measure->tick();
    double prevX = 0.0;
    for (const Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->tick() == tick) {
            return s->x();
        }
        if (s->tick() > tick) {
            return prevX + (s->x() - prevX) * ((tick - prevTick) / (s->tick() - prevTick)).toDouble();
        }
        prevTick = s->tick();
        prevX = s->x();
    }

    const Fraction endTick = measure->endTick();
    if (endTick <= prevTick) {
        return measure->width();
    }
    return prevX + (measure->width() - prevX) * ((tick - prevTick) / (endTick - prevTick)).toDouble();
}

struct BracketEnd {
    double x = 0.0;
    // false where it is open-ended
    bool closed = true;
};

// where the bracket of a tuplet of `ticks` ends: at the end of its time, or open-ended at the barline or the end of the
// enclosing tuplet
static BracketEnd bracketEnd(const TupletPreview& tuplet, const Fraction& ticks, const Measure* measure, double x, bool beyondScore)
{
    const double spatium = measure->score()->style().spatium();
    if (tuplet.waiting) {
        return { x, true };
    }
    if (beyondScore) {
        return { x + BEYOND_SCORE_LENGTH * spatium, false };
    }

    const bool closed = ticks <= tuplet.space;
    const double endX = measure->canvasPos().x() + measureXAt(measure, tuplet.tick + (closed ? ticks : tuplet.space));
    return { std::max(x, closed ? endX - END_GAP * spatium : endX), closed };
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
    // where it doesn't fit, a lightened critical colour; never while it isn't sized yet
    if (!tuplet->fits && !tuplet->waiting) {
        color = Color::fromQColor(engravingConfiguration()->criticalColor().toQColor().lighter(135));
    }
    painter->setPen(color);

    // the number or ratio as set for the tuplet; just the number if the score hides it
    const TupletPreview::Level& level = tuplet->levels.back();
    const TupletNumberType numberType = level.numberType == TupletNumberType::NO_TEXT
                                        ? TupletNumberType::SHOW_NUMBER : level.numberType;
    const String digits = Tuplet::numberText(level.ratio, numberType, /*useSymbols*/ false);

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

    // once sized, a bracket with the number in the middle; in tight spacing, only the number
    const BracketEnd end = bracketEnd(*tuplet, level.ticks, measure, x, state.beyondScore());
    const double hook = HOOK_LENGTH * spatium;
    const double xCentre = (x + end.x) * 0.5;
    const double xGapLeft = xCentre - ink.width() * scale * 0.5 - NUMBER_GAP * spatium;
    const double xGapRight = xCentre + ink.width() * scale * 0.5 + NUMBER_GAP * spatium;
    const bool bracket = !tuplet->waiting && xGapLeft - x >= hook * 0.5 && end.x - xGapRight >= hook * 0.5;

    // left-aligned at the input position, or in the middle of the bracket
    const double numberX = bracket ? xCentre - ink.center().x() * scale : x - (useSymbols ? ink.left() * scale : 0.0);
    if (useSymbols) {
        musicFont->draw(symbols, painter, mag * scale, PointF(numberX, baseline));
    } else {
        font.setPointSizeF(font.pointSizeF() * scale);
        painter->setFont(font);
        painter->drawText(PointF(numberX, baseline), digits);
    }

    if (!bracket) {
        return;
    }

    // the hooks point to the staff
    const double yHook = state.voice() % 2 == 0 ? centreY + hook : centreY - hook;
    Pen pen(color, LINE_WIDTH * spatium);
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setPen(pen);
    const PointF left[] = { PointF(x, yHook), PointF(x, centreY), PointF(xGapLeft, centreY) };
    const PointF right[] = { PointF(xGapRight, centreY), PointF(end.x, centreY), PointF(end.x, yHook) };
    painter->drawPolyline(left, 3);
    // open-ended where it runs on
    painter->drawPolyline(right, end.closed ? 3 : 2);
}
