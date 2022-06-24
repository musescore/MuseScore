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

#include "accessibility/accessibleitem.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"

#include "log.h"

using namespace mu::draw;
using namespace mu::engraving;

static const mu::draw::Color DEBUG_ELTREE_SELECTED_COLOR(164, 0, 0);

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

    if ((isDiagnosticSelected || configuration()->debuggingOptions().showBoundingRect)
        && !item->bbox().isEmpty()) {
        double scaling = painter.worldTransform().m11() / configuration()->guiScaling();
        draw::Pen borderPen(DEBUG_ELTREE_SELECTED_COLOR, (item->selected() ? 2.0 : 1.0) / scaling);

        // Draw bbox
        painter.setPen(borderPen);
        painter.setBrush(draw::BrushStyle::NoBrush);
        painter.drawRect(item->bbox());
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

    if (options.showSystemBoundingRect) {
        painter.setBrush(BrushStyle::NoBrush);
        painter.setPen(Pen(Color::black, 3.0 / scaling));

        for (const System* system : page->systems()) {
            PointF pt(system->ipos());
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

                PointF pt(system->ipos().x(), system->ipos().y() + ss->y());
                painter.translate(pt);
                ss->skyline().paint(painter, 3.0 / scaling);
                painter.translate(-pt);
            }
        }
    }

    if (options.showSegmentShapes) {
        painter.setBrush(BrushStyle::NoBrush);
        painter.setPen(Pen(Color(238, 238, 144), 2.0 / scaling));

        for (const System* system : page->systems()) {
            for (const MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }

                const Measure* m = toMeasure(mb);

                for (const Segment* s = m->first(); s; s = s->next()) {
                    for (size_t i = 0; i < score->nstaves(); ++i) {
                        PointF pt(s->pos().x() + m->pos().x() + system->pos().x(),
                                  system->staffYpage(static_cast<int>(i)));
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
        painter.setPen(Pen(Color::redColor, 4.0));
        painter.setBrush(BrushStyle::NoBrush);

        double _spatium = score->spatium();

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
