/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "debugpaint.h"

#include "paintdebugger.h"

//#include "accessibility/accessibleitem.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/system.h"
#include "dom/measurebase.h"
#include "dom/measure.h"
#include "dom/segment.h"

#include "tdraw.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static const Color DEBUG_ELTREE_SELECTED_COLOR(164, 0, 0);

/// Generates a seemingly random but stable color based on a pointer address.
/// If we would use really random colors, they would change on every redraw.
/// Additional advantage: being able to see when an element is replaced with
/// a different element during layout.
static Color colorForPointer(const void* ptr)
{
    static constexpr uint32_t rgbTotalMax = 600;
    static constexpr uint32_t mask = 0xFFFF;
    static constexpr double max = mask + 1.0;

    auto hash = std::hash<const void*> {}(ptr);
    uint32_t cmpnt1 = hash & mask;
    uint32_t cmpnt2 = (hash >> 16) & mask;

    uint32_t r = std::clamp(uint32_t(cmpnt1 / max * 255), 30u, 255u);
    uint32_t g = std::clamp(uint32_t(cmpnt2 / max * 255), 30u, 255u);
    uint32_t b = std::clamp(rgbTotalMax - r - g,          30u, 255u);

    return Color(r, g, b, 128);
}

void DebugPaint::paintElementDebug(Painter& painter, const EngravingItem* item)
{
    // Elements tree
    bool isDiagnosticSelected = item->score()->elementsProvider() ? item->score()->elementsProvider()->isSelected(item) : false;

    PointF pos(item->pagePos());
    painter.translate(pos);

    RectF bbox = item->ldata()->bbox();

    if (item->isType(ElementType::SEGMENT)) {
        if (muse::RealIsNull(bbox.height())) {
            bbox.setHeight(10.0);
            LOGD() << "Segment bbox height is null";
        }
    }

    if (!bbox.isEmpty()) {
        // Draw shape
        if (item->configuration()->debuggingOptions().colorElementShapes
            && !item->isPage() && !item->isSystem() && !item->isStaffLines() && !item->isBox()) {
            PainterPath path;
            path.setFillRule(PainterPath::FillRule::WindingFill);

            Shape shape = item->shape();
            for (const RectF& rect : shape.elements()) {
                path.addRect(rect);
            }

            painter.setPen(PenStyle::NoPen);
            painter.setBrush(colorForPointer(item));
            painter.drawPath(path);
        }

        if (item->configuration()->debuggingOptions().showElementMasks) {
            PainterPath path;
            path.setFillRule(PainterPath::FillRule::WindingFill);
            for (const ShapeElement& el : item->ldata()->mask().elements()) {
                path.addRect(el);
            }

            painter.setPen(Color::BLACK);
            Brush brush(Color::BLACK);
            brush.setStyle(BrushStyle::BDiagPattern);
            painter.setBrush(brush);
            painter.drawPath(path);
        }

        // Draw bbox
        if (isDiagnosticSelected || item->configuration()->debuggingOptions().showElementBoundingRects) {
            double scaling = painter.worldTransform().m11() / item->configuration()->guiScaling();
            Pen borderPen(DEBUG_ELTREE_SELECTED_COLOR, (item->selected() ? 2.0 : 1.0) / scaling);

            painter.setPen(borderPen);
            painter.setBrush(BrushStyle::NoBrush);
            painter.drawRect(bbox);
        }
    }

    painter.translate(-pos);
}

void DebugPaint::paintPageDebug(Painter& painter, const Page* page, const std::vector<EngravingItem*>& items)
{
    if (items.empty()) {
        return;
    }

    auto options = items.front()->configuration()->debuggingOptions();
    if (!options.anyEnabled()) {
        return;
    }

    IF_ASSERT_FAILED(page) {
        return;
    }

    Score* score = page->score();

    IF_ASSERT_FAILED(score) {
        return;
    }

    double scaling = painter.worldTransform().m11() / items.front()->configuration()->guiScaling();

    painter.save();

    for (const EngravingItem* item : items) {
        paintElementDebug(painter, item);
    }

    if (options.showSystemBoundingRects) {
        painter.setBrush(BrushStyle::NoBrush);
        painter.setPen(Pen(Color::BLACK, 3.0 / scaling));

        for (const System* system : page->systems()) {
            PointF pt(system->ldata()->pos());
            double h = system->height() + system->minBottom() + system->minTop();
            painter.translate(pt);
            RectF rect(0.0, -system->minTop(), system->width(), h);
            painter.drawRect(rect);
            painter.translate(-pt);
        }
    }

    if (options.showSkylines) {
        for (const System* system : page->systems()) {
            for (const SysStaff* ss : system->staves()) {
                if (!ss->show()) {
                    continue;
                }

                PointF pt(system->ldata()->pos().x(), system->ldata()->pos().y() + ss->y());
                painter.translate(pt);
                ss->skyline().paint(painter, 3);
                painter.translate(-pt);
            }
        }
    }

    if (options.colorSegmentShapes) {
        painter.setPen(PenStyle::NoPen);

        for (const System* system : page->systems()) {
            for (const MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }

                const Measure* m = toMeasure(mb);

                for (const Segment* s = m->first(); s; s = s->next()) {
                    if (!s->enabled()) {
                        continue;
                    }

                    painter.setBrush(colorForPointer(s));

                    for (staff_idx_t i = 0; i < score->nstaves(); ++i) {
                        PainterPath path;
                        path.setFillRule(PainterPath::FillRule::WindingFill);

                        Shape shape = s->shapes().at(i);
                        for (const RectF& rect : shape.elements()) {
                            path.addRect(rect);
                        }

                        PointF pt(s->pos().x() + m->pos().x() + system->pos().x(),
                                  system->staffYpage(i));
                        painter.translate(pt);
                        painter.drawPath(path);
                        painter.translate(-pt);
                    }
                }
            }
        }
    }

    if (options.showSegmentShapes) {
        painter.setBrush(BrushStyle::NoBrush);
        Pen pen(Color(238, 238, 144), 2.0 / scaling);

        for (const System* system : page->systems()) {
            for (const MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }

                const Measure* m = toMeasure(mb);

                for (const Segment* s = m->first(); s; s = s->next()) {
                    pen.setStyle(s->enabled() ? PenStyle::SolidLine : PenStyle::DotLine);
                    painter.setPen(pen);

                    for (staff_idx_t i = 0; i < score->nstaves(); ++i) {
                        PointF pt(s->pos().x() + m->pos().x() + system->pos().x(),
                                  system->staffYpage(i));
                        painter.translate(pt);
                        s->shapes().at(i).paint(painter);
                        painter.translate(-pt);
                    }
                }
            }
        }
    }

    if (options.showCorruptedMeasures && score->hasCorruptedMeasures()) {
        painter.setPen(Pen(Color::RED, 4.0));
        painter.setBrush(BrushStyle::NoBrush);

        double _spatium = score->style().spatium();

        for (const System* system : page->systems()) {
            for (const MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }

                const Measure* m = toMeasure(mb);
                for (size_t staffIdx = 0; staffIdx < m->score()->nstaves(); staffIdx++) {
                    if (m->corrupted(staffIdx)) {
                        painter.drawRect(m->staffPageBoundingRect(staffIdx).adjusted(0, -_spatium, 0, _spatium));
                    }
                }
            }
        }
    }

    painter.restore();
}

void DebugPaint::paintTreeElement(Painter& painter, const EngravingItem* item)
{
//    if (item->ldata()->isSkipDraw()) {
//        return;
//    }
    item->itemDiscovered = false;
    PointF itemPosition(item->pagePos());

    painter.translate(itemPosition);
    TDraw::drawItem(item, &painter);
    painter.translate(-itemPosition);
}

static void paintRecursive(Painter& painter, const EngravingItem* item)
{
    DebugPaint::paintTreeElement(painter, item);

    for (const EngravingItem* eItem : item->childrenItems()) {
        paintRecursive(painter, eItem);
    }
}

void DebugPaint::paintPageTree(Painter& painter, const Page* page)
{
    painter.save();

    paintRecursive(painter, page);

    painter.restore();
}
