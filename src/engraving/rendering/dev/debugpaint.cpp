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
#include "debugpaint.h"

#include "paintdebugger.h"

//#include "accessibility/accessibleitem.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"

#include "log.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

static const mu::draw::Color DEBUG_ELTREE_SELECTED_COLOR(164, 0, 0);

/// Generates a seemingly random but stable color based on a pointer address.
/// (If we would use really random colors, the would change on every redraw.)
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

void DebugPaint::paintElementDebug(mu::draw::Painter& painter, const EngravingItem* item,
                                   std::shared_ptr<PaintDebugger>& debugger)
{
    // Elements tree
    bool isDiagnosticSelected = elementsProvider()->isSelected(item);
    if (isDiagnosticSelected) {
        // Overriding pen
        debugger->setDebugPenColor(DEBUG_ELTREE_SELECTED_COLOR);
    }

    PointF pos(item->pagePos());
    painter.translate(pos);

    if (!item->layoutData()->bbox().isEmpty()) {
        // Draw shape
        if (configuration()->debuggingOptions().colorElementShapes
            && !item->isPage() && !item->isSystem() && !item->isStaffLines() && !item->isBox()) {
            PainterPath path;
            path.setFillRule(PainterPath::FillRule::WindingFill);

            Shape shape = item->shape();
            for (const RectF& rect : shape) {
                path.addRect(rect);
            }

            painter.setPen(PenStyle::NoPen);
            painter.setBrush(colorForPointer(item));
            painter.drawPath(path);
        }

        // Draw bbox
        if (isDiagnosticSelected || configuration()->debuggingOptions().showElementBoundingRects) {
            double scaling = painter.worldTransform().m11() / configuration()->guiScaling();
            draw::Pen borderPen(DEBUG_ELTREE_SELECTED_COLOR, (item->selected() ? 2.0 : 1.0) / scaling);

            painter.setPen(borderPen);
            painter.setBrush(draw::BrushStyle::NoBrush);
            painter.drawRect(item->layoutData()->bbox());
        }
    }

    painter.translate(-pos);

    debugger->restorePenColor();
}

void DebugPaint::paintElementsDebug(mu::draw::Painter& painter, const std::vector<EngravingItem*>& elements)
{
    // Setup debug provider
    auto originalProvider = painter.provider();
    std::shared_ptr<PaintDebugger> debugger = std::make_shared<PaintDebugger>(originalProvider);
    painter.setProvider(debugger, false);

    for (const EngravingItem* element : elements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElementDebug(painter, element, debugger);
    }

    // Restore provider
    debugger->restorePenColor();
    painter.setProvider(debugger->realProvider(), false);
}

void DebugPaint::paintPageDebug(Painter& painter, const Page* page)
{
    IF_ASSERT_FAILED(page) {
        return;
    }

    Score* score = page->score();

    IF_ASSERT_FAILED(score) {
        return;
    }

    double scaling = painter.worldTransform().m11() / configuration()->guiScaling();

    painter.save();

    auto options = configuration()->debuggingOptions();

    if (options.showSystemBoundingRects) {
        painter.setBrush(BrushStyle::NoBrush);
        painter.setPen(Pen(Color::BLACK, 3.0 / scaling));

        for (const System* system : page->systems()) {
            PointF pt(system->layoutData()->pos());
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

                PointF pt(system->layoutData()->pos().x(), system->layoutData()->pos().y() + ss->y());
                painter.translate(pt);
                ss->skyline().paint(painter, 3.0 / scaling);
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
                        for (const RectF& rect : shape) {
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

#ifndef NDEBUG
    if (options.showCorruptedMeasures) {
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
                        painter.drawRect(m->staffabbox(staffIdx).adjusted(0, -_spatium, 0, _spatium));
                    }
                }
            }
        }
    }
#endif

    painter.restore();
}
