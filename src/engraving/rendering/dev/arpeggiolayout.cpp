/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "arpeggiolayout.h"

#include "libmscore/arpeggio.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void ArpeggioLayout::layout(const Arpeggio* item, const LayoutContext& ctx, Arpeggio::LayoutData* data)
{
    if (ctx.conf().styleB(Sid::ArpeggioHiddenInStdIfTab)) {
        if (item->staff() && item->staff()->isPitchedStaff(item->tick())) {
            for (Staff* s : item->staff()->staffList()) {
                if (s->onSameScore(item) && s->isTabStaff(item->tick()) && s->visible()) {
                    data->bbox = RectF();
                    return;
                }
            }
        }
    }

    data->top = calcTop(item, ctx);
    data->bottom = calcBottom(item, ctx);

    data->mag = item->staff() ? item->staff()->staffMag(item->tick()) : item->mag();
    data->magS = ctx.conf().magS(data->mag);

    const IEngravingFontPtr font = ctx.engravingFont();
    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL: {
        ArpeggioLayout::symbolLine(font, data, SymId::wiggleArpeggiatoUp, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        data->symsBBox = font->bbox(data->symbols, data->magS);
        data->bbox = RectF(0.0, -data->symsBBox.x() + data->top, data->symsBBox.height(), data->symsBBox.width());
    } break;

    case ArpeggioType::UP: {
        ArpeggioLayout::symbolLine(font, data, SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        data->symsBBox = font->bbox(data->symbols, data->magS);
        data->bbox = RectF(0.0, -data->symsBBox.x() + data->top, data->symsBBox.height(), data->symsBBox.width());
    } break;

    case ArpeggioType::DOWN: {
        ArpeggioLayout::symbolLine(font, data, SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated +90 degrees (so that UpArrow turns into a DownArrow)
        data->symsBBox = font->bbox(data->symbols, data->magS);
        data->bbox = RectF(0.0, data->symsBBox.x() + data->top, data->symsBBox.height(), data->symsBBox.width());
    } break;

    case ArpeggioType::UP_STRAIGHT: {
        double x1 = item->spatium() * 0.5;
        data->symsBBox = font->bbox(SymId::arrowheadBlackUp, data->magS);
        double w = data->symsBBox.width();
        data->bbox = RectF(x1 - w * 0.5, data->top, w, data->bottom);
    } break;

    case ArpeggioType::DOWN_STRAIGHT: {
        double x1 = item->spatium() * 0.5;
        data->symsBBox = font->bbox(SymId::arrowheadBlackDown, data->magS);
        double w = data->symsBBox.width();
        data->bbox = RectF(x1 - w * 0.5, data->top, w, data->bottom);
    } break;

    case ArpeggioType::BRACKET: {
        double w  = ctx.conf().styleS(Sid::ArpeggioHookLen).val() * item->spatium();
        data->bbox = RectF(0.0, data->top, w, data->bottom);
    } break;
    }
}

//   layoutArpeggio2
//    called after layout of page

void ArpeggioLayout::layoutArpeggio2(Arpeggio* item, LayoutContext& ctx)
{
    if (!item || item->span() < 2) {
        return;
    }
    computeHeight(item, /*includeCrossStaffHeight = */ true);
    TLayout::layout(item, ctx);
}

void ArpeggioLayout::computeHeight(Arpeggio* item, bool includeCrossStaffHeight)
{
    Chord* topChord = item->chord();
    if (!topChord) {
        return;
    }
    double y = topChord->upNote()->pagePos().y() - topChord->upNote()->headHeight() * .5;

    Note* bottomNote = topChord->downNote();
    if (includeCrossStaffHeight) {
        track_idx_t bottomTrack = item->track() + (item->span() - 1) * VOICES;
        EngravingItem* element = topChord->segment()->element(bottomTrack);
        Chord* bottomChord = (element && element->isChord()) ? toChord(element) : topChord;
        bottomNote = bottomChord->downNote();
    }

    double h = bottomNote->pagePos().y() + bottomNote->headHeight() * .5 - y;
    item->setHeight(h);
}

void ArpeggioLayout::layoutOnEditDrag(Arpeggio* item, LayoutContext& ctx)
{
    ArpeggioLayout::layout(item, ctx, item->mutLayoutData());
}

void ArpeggioLayout::layoutOnEdit(Arpeggio* item, LayoutContext& ctx)
{
    ArpeggioLayout::layout(item, ctx, item->mutLayoutData());

    Chord* c = item->chord();
    item->setPosX(-(item->width() + item->spatium() * .5));

    layoutArpeggio2(c->arpeggio(), ctx);
    Fraction _tick = item->tick();

    ctx.setLayout(_tick, _tick, item->staffIdx(), item->staffIdx() + item->span(), item);
}

//---------------------------------------------------------
//   symbolLine
//    construct a string of symbols approximating width w
//---------------------------------------------------------
void ArpeggioLayout::symbolLine(const IEngravingFontPtr& f, Arpeggio::LayoutData* data, SymId end, SymId fill)
{
    double w = data->bottom - data->top;
    double w1 = f->advance(end, data->magS);
    double w2 = f->advance(fill, data->magS);
    int n = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        data->symbols.push_back(fill);
    }
    data->symbols.push_back(end);
}

double ArpeggioLayout::calcTop(const Arpeggio* item, const LayoutContext& ctx)
{
    double top = -item->userLen1();
    if (!item->explicitParent()) {
        return top;
    }

    switch (item->arpeggioType()) {
    case ArpeggioType::BRACKET: {
        double lineWidth = ctx.conf().styleMM(Sid::ArpeggioLineWidth);
        return top - lineWidth / 2.0;
    }
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    case ArpeggioType::DOWN: {
        // if the top is in the staff on a space, move it up
        // if the bottom note is on a line, the distance is 0.25 spaces
        // if the bottom note is on a space, the distance is 0.5 spaces
        int topNoteLine = item->chord()->upNote()->line();
        int lines = item->staff()->lines(item->tick());
        int bottomLine = (lines - 1) * 2;
        if (topNoteLine <= 0 || topNoteLine % 2 == 0 || topNoteLine >= bottomLine) {
            return top;
        }
        int downNoteLine = item->chord()->downNote()->line();
        if (downNoteLine % 2 == 1 && downNoteLine < bottomLine) {
            return top - 0.4 * item->spatium();
        }
        return top - 0.25 * item->spatium();
    }
    default: {
        return top - item->spatium() / 4;
    }
    }
}

double ArpeggioLayout::calcBottom(const Arpeggio* item, const LayoutContext& ctx)
{
    double top = -item->userLen1();
    double bottom = item->height() + item->userLen2();
    if (!item->explicitParent()) {
        return bottom;
    }

    switch (item->arpeggioType()) {
    case ArpeggioType::BRACKET: {
        double lineWidth = ctx.conf().styleMM(Sid::ArpeggioLineWidth);
        return bottom - top + lineWidth;
    }
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    case ArpeggioType::DOWN: {
        return bottom;
    }
    default: {
        return bottom - top + item->spatium() / 2;
    }
    }
}
