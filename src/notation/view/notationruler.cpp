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

#include "notationruler.h"

#include "notation/notationtypes.h"
#include "engraving/dom/sig.h"

#include "ui/view/iconcodes.h"
#include "draw/types/color.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

void NotationRuler::paint(Painter* painter, const NoteInputState& state)
{
    TRACEFUNC;

    if (!state.isValid()) {
        return;
    }

    const Staff* staff = state.staff();
    const Segment* currSegment = state.segment();
    const System* system = currSegment->system();
    const Measure* measure = currSegment->measure();
    const SysStaff* sysStaff = system ? system->staff(state.staffIdx()) : nullptr;
    const TimeSignature* timeSig = staff ? staff->timeSig(state.tick()) : nullptr;

    if (!measure || !sysStaff || !timeSig) {
        return;
    }

    const muse::PointF measurePos = measure->canvasPos();
    const int inputTicks = state.tick().ticks();
    const int measureTicks = measure->tick().ticks();
    const int ticksBeat = mu::engraving::ticks_beat(timeSig->denominator());
    const int subdivisionTicks = ticksBeat / 2;

    std::map<size_t, const Segment*> lineToSegment;

    for (const Segment& segment : measure->segments()) {
        if (!segment.isChordRestType()) {
            continue;
        }

        const int segmentTicks = segment.tick().ticks();
        if (segmentTicks % subdivisionTicks != 0) {
            continue; // this segment does not correspond to any line
        }

        const size_t line = (segmentTicks - measureTicks) / subdivisionTicks;
        lineToSegment[line] = &segment;
    }

    const size_t lineCount = timeSig->numerator() * 2;
    const double spatium = staff->score()->style().spatium();
    const double aboveStaffSpacing = spatium * 2;
    const double lineY = measurePos.y() + sysStaff->y() - aboveStaffSpacing;

    size_t prevAccurateLine = muse::nidx;
    bool currentPositionPainted = false;

    for (size_t i = 0; i < lineCount; ++i) {
        const int lineTicks = measureTicks + subdivisionTicks * i;
        double lineX = 0.0;

        auto it = lineToSegment.find(i);
        if (it != lineToSegment.end()) {
            lineX = measurePos.x() + it->second->x();
            prevAccurateLine = i;
        } else if (prevAccurateLine != muse::nidx) {
            auto prevIt = lineToSegment.find(prevAccurateLine);
            auto nextIt = std::next(prevIt);

            if (nextIt != lineToSegment.end()) {
                const size_t dist = nextIt->first - prevIt->first;
                const double segmentWidth = (nextIt->second->x() - prevIt->second->x()) / static_cast<double>(dist);
                lineX = measurePos.x() + prevIt->second->x() + segmentWidth * (i % dist);
            } else {
                const size_t dist = lineCount - prevIt->first;
                const double segmentWidth = (measure->width() - prevIt->second->x()) / static_cast<double>(dist);
                lineX = measurePos.x() + prevIt->second->x() + segmentWidth * (i % dist);
            }
        } else {
            UNREACHABLE;
            continue;
        }

        const LineType type = lineType(lineTicks, inputTicks, i);
        paintLine(painter, type, PointF(lineX, lineY), spatium, state.voice());
        currentPositionPainted |= type == LineType::CurrentPosition;
    }

    if (!currentPositionPainted) {
        const double lineX = measurePos.x() + currSegment->x();
        paintLine(painter, LineType::CurrentPosition, PointF(lineX, lineY), spatium, state.voice());
    }
}

NotationRuler::LineType NotationRuler::lineType(int lineTicks, int inputTicks, size_t lineIdx)
{
    if (lineTicks == inputTicks) {
        return LineType::CurrentPosition;
    }

    return lineIdx % 2 == 0 ? LineType::MainBeat : LineType::Subdivision;
}

void NotationRuler::paintLine(Painter* painter, LineType type, const PointF& point, double spatium, voice_idx_t voiceIdx)
{
    muse::RectF rect;
    Color color = configuration()->selectionColor(voiceIdx);

    switch (type) {
    case LineType::CurrentPosition:
        rect.setHeight(spatium * 4.);
        rect.setWidth(spatium * 2.);
        rect.setX(point.x() - rect.width() / 2);
        color.setAlpha(192);
        break;
    case LineType::MainBeat:
        rect.setWidth(std::clamp(spatium / 6., 1., 4.));
        rect.setHeight(spatium * 2.);
        rect.setX(point.x());
        color.setAlpha(192);
        break;
    case LineType::Subdivision:
        rect.setWidth(std::clamp(spatium / 6., 1., 4.));
        rect.setHeight(spatium);
        rect.setX(point.x());
        color.setAlpha(128);
        break;
    }

    rect.setY(point.y() - rect.height());

    if (type == LineType::CurrentPosition) {
        Font font(uiConfiguration()->iconsFontFamily(), Font::Type::Icon);
        font.setPixelSize(rect.height() * 0.7);

        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, AlignHCenter | AlignBottom,
                          Char(static_cast<char16_t>(muse::ui::IconCode::Code::DURATION_CURSOR)));
    } else {
        painter->fillRect(rect, color);
    }
}
