/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "markerlayout.h"
#include "layoutcontext.h"
#include "tlayout.h"
#include "autoplace.h"

#include "../dom/marker.h"

using namespace mu::engraving::rendering::score;

void MarkerLayout::layoutMarker(const Marker* item, TextBase::LayoutData* ldata, LayoutContext& ctx)
{
    doLayoutMarker(item, ldata, ctx);

    double customTextOffset = computeCustomTextOffset(item, ldata, ctx);
    ldata->moveX(-customTextOffset);

    Measure* measure = item->parentItem() ? toMeasure(item->parentItem()) : nullptr;
    if (measure && item->autoplace()) {
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(ldata->isSetBbox());
        LD_CONDITION(measure->ldata()->isSetPos());
    }

    Autoplace::autoplaceMeasureElement(item, ldata);
}

void MarkerLayout::doLayoutMarker(const Marker* item, TextBase::LayoutData* ldata, LayoutContext& ctx)
{
    Measure* measure = toMeasure(item->parentItem());
    IF_ASSERT_FAILED(measure) {
        LOGD() << "Marker has no measure";
    }
    LD_CONDITION(measure->ldata()->isSetBbox());

    TLayout::layoutBaseTextBase(item, ldata);

    // POSITION
    bool rightMarker = item->isRightMarker();
    double xAdj = 0.0;
    double blWidth = 0.0;
    if (rightMarker) {
        xAdj = measure ? measure->width() : 0.0;
    }

    bool startRepeat = rightMarker ? measure->nextMeasure() && measure->nextMeasure()->repeatStart() : measure->repeatStart();
    bool endRepeat = rightMarker ? measure->repeatEnd() : measure->prevMeasure() && measure->prevMeasure()->repeatEnd();

    if (startRepeat) {
        blWidth = -ctx.conf().styleS(Sid::endBarWidth).toMM(item->spatium());
    } else if (endRepeat) {
        blWidth = ctx.conf().styleS(Sid::endBarWidth).toMM(item->spatium());
    } else if ((measure->isFirstInSystem() || (measure->prev() && !measure->prev()->isMeasure())) && !rightMarker) {
        // Start of score
        const BarLine* bl =  measure->startBarLine();
        blWidth = bl ? -bl->width() : 0.0;
    } else {
        Measure* blMeasure = rightMarker ? measure : measure->prevMeasure();
        const BarLine* bl = blMeasure ? blMeasure->endBarLine() : nullptr;
        blWidth = bl ? bl->width() : 0.0;
    }

    AlignH hPos = item->centerOnSymbol()
                  && !item->symbolString().empty() ? AlignH::HCENTER : item->getProperty(Pid::POSITION).value<AlignH>();
    switch (hPos) {
    case AlignH::HCENTER:
        xAdj -= (ldata->bbox().width() + blWidth) / 2;
        break;
    case AlignH::RIGHT:
        xAdj -= ldata->bbox().width() + (startRepeat ? blWidth : 0.0);
        break;
    case AlignH::LEFT:
        xAdj -= startRepeat ? 0.0 : blWidth;
        break;
    }

    ldata->moveX(xAdj);
}

double MarkerLayout::computeCustomTextOffset(const Marker* item, TextBase::LayoutData* ldata, LayoutContext& ctx)
{
    String referenceString = item->symbolString();

    bool centerOnSymbol = item->centerOnSymbol() && !referenceString.empty();
    if (!centerOnSymbol) {
        return 0.0;
    }

    if (item->xmlText() == referenceString) {
        return 0.0;
    }

    Marker referenceMarker(*item);
    referenceMarker.setXmlText(referenceString);
    doLayoutMarker(&referenceMarker, referenceMarker.mutldata(), ctx);

    TextFragment referenceFragment;
    if (!referenceMarker.ldata()->blocks.empty()) {
        TextBlock referenceBlock = referenceMarker.ldata()->blocks.front();
        if (!referenceBlock.fragments().empty()) {
            referenceFragment = referenceMarker.ldata()->blocks.front().fragments().front();
        }
    }

    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& fragment : block.fragments()) {
            if (fragment.text == referenceFragment.text) {
                return ldata->pos().x() + fragment.pos.x() - referenceMarker.ldata()->pos().x() + referenceFragment.pos.x();
            }
        }
    }

    return 0.0;
}
