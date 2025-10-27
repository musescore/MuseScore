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

#include "boxlayout.h"
#include "dom/fret.h"
#include "dom/harmony.h"
#include "dom/system.h"
#include "dom/text.h"
#include "draw/fontmetrics.h"

#include "editing/editfretboarddiagram.h"
#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using namespace muse::draw;

void BoxLayout::layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    switch (item->type()) {
    case ElementType::HBOX:
        BoxLayout::layoutHBox(static_cast<const HBox*>(item), static_cast<HBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::VBOX:
        BoxLayout::layoutVBox(static_cast<const VBox*>(item), static_cast<VBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FBOX:
        BoxLayout::layoutFBox(static_cast<const FBox*>(item), static_cast<FBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TBOX:
        BoxLayout::layoutTBox(static_cast<const TBox*>(item), static_cast<TBox::LayoutData*>(ldata), ctx);
        break;
    default:
        UNREACHABLE;
        break;
    }
}

void BoxLayout::layoutBaseBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    TLayout::layoutBaseMeasureBase(item, ldata, ctx);
}

void BoxLayout::layoutHBox(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx)
{
    if (item->explicitParent() && item->explicitParent()->isVBox()) {
        const VBox* parentVBox = toVBox(item->explicitParent());

        LD_CONDITION(parentVBox->ldata()->isSetBbox());

        double x = parentVBox->leftMargin() * DPMM;
        double y = parentVBox->topMargin() * DPMM;
        double w = item->absoluteFromSpatium(item->boxWidth());
        double h = parentVBox->ldata()->bbox().height() - (parentVBox->topMargin() + parentVBox->bottomMargin()) * DPMM;
        ldata->setPos(x, y);
        ldata->setBbox(0.0, 0.0, w, h);
    } else if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->ldata()->isSetBbox());

        if (!ldata->isSetPos()) {
            ldata->setPos(PointF());
        }
        ldata->setBbox(item->absoluteFromSpatium(item->topGap()), 0.0, item->absoluteFromSpatium(item->boxWidth()),
                       parentSystem->ldata()->bbox().height());
    } else {
        ldata->setPos(PointF());
        ldata->setBbox(0.0, 0.0, 50, 50);
    }
    layoutBaseBox(item, ldata, ctx);
}

void BoxLayout::layoutHBox2(HBox* item, const LayoutContext& ctx)
{
    layoutBaseBox(item, item->mutldata(), ctx);
}

void BoxLayout::layoutVBox(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->setPos(PointF());

    if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->ldata()->isSetBbox());

        ldata->setBbox(0.0, 0.0, parentSystem->ldata()->bbox().width(), item->absoluteFromSpatium(item->boxHeight()));
    } else {
        ldata->setBbox(0.0, 0.0, 50, 50);
    }

    for (EngravingItem* e : item->el()) {
        TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
    }
    bool boxAutoSize = item->getProperty(Pid::BOX_AUTOSIZE).toBool();
    bool heightChanged = false;
    if (boxAutoSize) {
        double contentHeight = item->contentRect().height();

        if (contentHeight < item->minHeight()) {
            contentHeight = item->minHeight();
        }

        ldata->setHeight(contentHeight);
        heightChanged = true;
    }

    if (boxAutoSize && MScore::noImages) {
        // adjustLayoutWithoutImages
        double calculatedVBoxHeight = 0;
        const int padding = item->sizeIsSpatiumDependent() ? ctx.conf().spatium() : ctx.conf().style().defaultSpatium();
        ElementList elist = item->el();
        for (EngravingItem* e : elist) {
            if (e->isText()) {
                Text* txt = toText(e);
                Text::LayoutData* txtLD = txt->mutldata();

                LD_CONDITION(txtLD->isSetBbox());

                RectF bbox = txtLD->bbox();
                bbox.moveTop(0.0);
                txtLD->setBbox(bbox);
                calculatedVBoxHeight += txtLD->bbox().height() + padding;
            }
        }

        ldata->setHeight(calculatedVBoxHeight);
        heightChanged = true;
    }
    if (heightChanged) {
        for (EngravingItem* e : item->el()) {
            TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
        }
    }
}

void BoxLayout::layoutFBox(const FBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx)
{
    Score* score = item->score();

    if (item->needsRebuild()) {
        /// FBox::init requires all chord symbols in the score have valid ldata
        /// Perform layout
        for (mu::engraving::Segment* segment = score->firstSegment(mu::engraving::SegmentType::ChordRest); segment;
             segment = segment->next1(mu::engraving::SegmentType::ChordRest)) {
            for (EngravingItem* segItem : segment->annotations()) {
                if (!segItem || !segItem->part()) {
                    continue;
                }
                if (!(segItem->isHarmony() || segItem->isFretDiagram())) {
                    continue;
                }

                Harmony* harmony = segItem->isHarmony() ? toHarmony(segItem) : toFretDiagram(segItem)->harmony();
                if (!harmony || harmony->harmonyType() != HarmonyType::STANDARD || harmony->ldata()->isValid()) {
                    continue;
                }

                TLayout::layoutHarmony(harmony, harmony->mutldata(), ctx);
            }
        }

        const_cast<FBox*>(item)->init();
        const_cast<FBox*>(item)->setNeedsRebuild(false);
    }

    const System* parentSystem = item->system();
    LD_CONDITION(parentSystem->ldata()->isSetBbox());

    ldata->setPos(PointF());

    std::vector<FretDiagram*> fretDiagrams;
    for (EngravingItem* element : item->el()) {
        if (!element || !element->isFretDiagram() || !element->visible()) {
            continue;
        }

        FretDiagram* diagram = toFretDiagram(element);
        if (!diagram->visible()) {
            //! NOTE: We need to layout the diagrams to get the harmony names to show in the UI
            TLayout::layoutItem(diagram, const_cast<LayoutContext&>(ctx));

            //! but we don't need to draw them, so let's add a skip
            diagram->mutldata()->setIsSkipDraw(true);
            diagram->harmony()->mutldata()->setIsSkipDraw(true);
            continue;
        }

        fretDiagrams.emplace_back(diagram);
    }

    //! NOTE: layout fret diagrams and calculate sizes

    const size_t totalDiagrams = fretDiagrams.size();
    double maxFretDiagramWidth = 0.0;

    for (size_t i = 0; i < totalDiagrams; ++i) {
        FretDiagram* fretDiagram = fretDiagrams[i];
        fretDiagram->setUserMag(item->diagramScale());

        Harmony* harmony = fretDiagram->harmony();
        harmony->mutldata()->setMag(item->textScale());

        TLayout::layoutItem(fretDiagram, const_cast<LayoutContext&>(ctx));

        //! reset the skip wich was added above
        fretDiagram->mutldata()->setIsSkipDraw(false);
        harmony->mutldata()->setIsSkipDraw(false);

        double width = fretDiagram->ldata()->bbox().width();
        maxFretDiagramWidth = std::max(maxFretDiagramWidth, width);
    }

    //! NOTE: table view layout

    const double spatium = item->spatium();
    const size_t chordsPerRow = item->chordsPerRow();
    const double rowGap = item->rowGap().val() * spatium;
    const double columnGap = item->columnGap().val() * spatium;

    //! The height of each row is determined by the height of the tallest cell in that row
    std::vector<double> rowHeights;
    std::vector<double> harmonyHeights;
    std::vector<double> harmonyBaselines;
    for (size_t i = 0; i < totalDiagrams; i += chordsPerRow) {
        size_t itemsInRow = std::min(chordsPerRow, totalDiagrams - i);
        double maxRowHeight = 0.0;
        double maxHarmonyHeight = 0.0;
        double harmonyBaseline = 0.0;

        for (size_t j = 0; j < itemsInRow; ++j) {
            FretDiagram* fretDiagram = fretDiagrams[i + j];

            RectF fretRect = fretDiagram->ldata()->bbox();

            const Harmony::LayoutData* harmonyLdata = fretDiagram->harmony()->ldata();
            RectF harmonyRect = harmonyLdata->bbox().translated(harmonyLdata->pos());

            double height = fretRect.united(harmonyRect).height();
            maxRowHeight = std::max(maxRowHeight, height);
            maxHarmonyHeight = std::max(maxHarmonyHeight, -harmonyRect.top());
            harmonyBaseline = std::min(harmonyBaseline, harmonyLdata->pos().y());
        }

        rowHeights.push_back(maxRowHeight);
        harmonyHeights.push_back(maxHarmonyHeight);
        harmonyBaselines.push_back(harmonyBaseline);
    }

    const double cellWidth = maxFretDiagramWidth;
    const size_t columns = std::min(totalDiagrams, chordsPerRow);

    const double totalTableWidth = cellWidth * columns + (columns - 1) * columnGap;

    AlignH alignH = item->contentHorizontalAlignment();
    const double leftMargin = item->getProperty(Pid::LEFT_MARGIN).toDouble() * spatium;
    const double rightMargin = item->getProperty(Pid::RIGHT_MARGIN).toDouble() * spatium;
    const double topMargin = item->getProperty(Pid::TOP_MARGIN).toDouble() * spatium;
    const double bottomMargin = item->getProperty(Pid::BOTTOM_MARGIN).toDouble() * spatium;

    const double width = item->system()->width();

    const double startX = alignH == AlignH::HCENTER
                          ? (width - totalTableWidth) / 2
                          : alignH == AlignH::RIGHT ? width - totalTableWidth : 0.0;
    const double startY = topMargin;

    double bottomY = 0.0;

    for (size_t i = 0; i < totalDiagrams; ++i) {
        FretDiagram* fretDiagram = fretDiagrams[i];
        Harmony* harmony = fretDiagram->harmony();

        size_t row = i / chordsPerRow;
        size_t col = i % chordsPerRow;

        size_t itemsInRow = std::min(chordsPerRow, totalDiagrams - row * chordsPerRow);
        double rowOffsetX = alignH == AlignH::HCENTER
                            ? (totalTableWidth - (itemsInRow * cellWidth + (itemsInRow - 1) * columnGap)) / 2
                            : alignH == AlignH::RIGHT
                            ? totalTableWidth - (itemsInRow * cellWidth + (itemsInRow - 1) * columnGap) - rightMargin + spatium
                            : leftMargin + spatium;

        double x = startX + rowOffsetX + col * (cellWidth + columnGap);

        double y = startY;
        for (size_t r = 0; r < row; ++r) {
            y += rowHeights[r] + rowGap;
        }

        double commonBaseline = harmonyBaselines[row];
        double thisBaseline = harmony->ldata()->pos().y();
        double baselineOff = commonBaseline - thisBaseline;
        harmony->mutldata()->moveY(baselineOff);

        double fretDiagramX = x;
        double fretDiagramY = y + harmonyHeights[row];

        fretDiagram->mutldata()->setPos(PointF(fretDiagramX, fretDiagramY));

        bottomY = std::max(bottomY, fretDiagram->mutldata()->bbox().translated(fretDiagram->mutldata()->pos()).bottom());
    }

    double height = bottomY + bottomMargin;
    if (muse::RealIsNull(height)) {
        height = item->minHeight();
    }

    ldata->setBbox(0.0, 0.0, width, height);
}

void BoxLayout::layoutTBox(const TBox* item, TBox::LayoutData* ldata, const LayoutContext& ctx)
{
    const System* parentSystem = item->system();

    LD_CONDITION(parentSystem->ldata()->isSetBbox());

    ldata->setPos(PointF());
    ldata->setBbox(0.0, 0.0, parentSystem->ldata()->bbox().width(), 0);

    TLayout::layoutText(item->text(), item->text()->mutldata());

    Text::LayoutData* textLD = item->text()->mutldata();

    double h = 0.0;
    if (item->text()->empty()) {
        h = FontMetrics::ascent(item->text()->font());
    } else {
        h = textLD->bbox().height();
    }
    double y = item->topMargin() * DPMM;
    textLD->setPos(item->leftMargin() * DPMM, y);
    h += item->topMargin() * DPMM + item->bottomMargin() * DPMM;
    ldata->setBbox(0.0, 0.0, item->system()->width(), h);

    TLayout::layoutBaseMeasureBase(item, ldata, ctx);   // layout LayoutBreak's
}
