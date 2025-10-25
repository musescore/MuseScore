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
 *
 */

#include "continuouspanel.h"

#include "engraving/rendering/score/tlayout.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafflines.h"
#include "engraving/dom/system.h"
#include "engraving/dom/text.h"
#include "engraving/dom/timesig.h"

#include "draw/painter.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;
using namespace mu::notation;
using namespace mu::engraving::rendering::score;

static constexpr bool ACCESSIBILITY_DISABLED = false;

ContinuousPanel::~ContinuousPanel()
{
    clearCache();
}

void ContinuousPanel::clearCache()
{
    delete m_cachedText;
    delete m_cachedName;
    delete m_cachedClef;
    delete m_cachedKeySig;
    delete m_cachedTimeSig;
    delete m_cachedBarLine;

    m_cachedText = nullptr;
    m_cachedName = nullptr;
    m_cachedClef = nullptr;
    m_cachedKeySig = nullptr;
    m_cachedTimeSig = nullptr;
    m_cachedBarLine = nullptr;
}

void ContinuousPanel::setNotation(INotationPtr notation)
{
    if (notation == m_notation) {
        return;
    }

    clearCache();
    m_width = 0;
    m_notation = notation;
}

void ContinuousPanel::paint(Painter& painter, const NotationViewContext& ctx, const engraving::rendering::PaintOptions& opt)
{
    TRACEFUNC;

    const mu::engraving::Score* score = this->score();
    if (!score) {
        return;
    }

    double offsetPanel = -ctx.xOffset / ctx.scaling;
    double y = 0;
    double oldWidth = 0;          // The last final panel width
    double newWidth = 0;          // New panel width
    double height = 0;
    double leftMarginTotal = 0;   // Sum of all elements left margin
    double panelRightPadding = 5;    // Extra space for the panel after last element

    const mu::engraving::Measure* measure = score->firstMeasure()->coveringMMRestOrThis();
    if (!measure) {
        return;
    }

    const mu::engraving::System* system = measure->system();
    if (!system) {
        return;
    }

    const double spatium = score->style().spatium();
    if (m_width <= 0) {
        m_width = measure->first()->x();
    }

    //
    // Set panel height for whole system
    //
    height = 6 * spatium;
    y = system->staffYpage(0) + system->page()->pos().y();
    double y2 = 0.0;
    const size_t staffCount = score->nstaves();
    for (mu::engraving::staff_idx_t i = 0; i < staffCount; ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->y() + ss->bbox().height();
    }
    height += y2 + 6 * spatium;
    y -= 6 * spatium;

    //
    // Check elements at current panel position
    //

    m_rect = RectF(offsetPanel + m_width, y, 1, height);

    mu::engraving::Page* page = score->pages().front();
    std::vector<mu::engraving::EngravingItem*> el = page->items(m_rect);
    if (el.empty()) {
        return;
    }

    std::stable_sort(el.begin(), el.end(), mu::engraving::elementLessThan);

    const mu::engraving::Measure* currentMeasure = nullptr;
    const bool showInvisible = score->isShowInvisible();
    for (const mu::engraving::EngravingItem* e : el) {
        e->itemDiscovered = false;
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isStaffLines()) {
            currentMeasure = toStaffLines(e)->measure();
            break;
        }
    }

    if (!currentMeasure) {
        return;
    }

    // Don't show panel if staff names are visible
    if (!ctx.fromLogical) {
        return;
    }

    const double canvasPosX = ctx.fromLogical(currentMeasure->canvasPos()).x();
    if (currentMeasure == score->firstMeasure() && canvasPosX > 0) {
        return;
    }

    double xPosMeasure = currentMeasure->canvasX();
    const double _measureWidth = currentMeasure->width();
    const engraving::Fraction tick = currentMeasure->tick();
    const engraving::Fraction currentTimeSigFraction = currentMeasure->timesig();

    //---------------------------------------------------------
    //   findElementWidths
    //      determines the max width for each element types
    //---------------------------------------------------------

    // The first pass serves to get the maximum width for each elements

    double lineWidthName = 0;
    double widthClef    = 0;
    double widthKeySig  = 0;
    double widthTimeSig = 0;
    double xPosTimeSig  = 0;

    // Ensure we have a cached segment to use as parent for temporary objects
    Segment* seg = score->tick2segmentMM(tick);

    // Initialize cached objects if needed
    if (!m_cachedText) {
        m_cachedText = engraving::Factory::createText(seg, mu::engraving::TextStyleType::DEFAULT,
                                                      ACCESSIBILITY_DISABLED);
        m_cachedText->setFlag(engraving::ElementFlag::MOVABLE, false);
        m_cachedText->setFamily(u"FreeSans");
        m_cachedText->setSizeIsSpatiumDependent(true);
    }

    if (!m_cachedName) {
        m_cachedName = engraving::Factory::createText(seg, mu::engraving::TextStyleType::DEFAULT,
                                                      ACCESSIBILITY_DISABLED);
        m_cachedName->setFlag(engraving::ElementFlag::MOVABLE, false);
        m_cachedName->setFamily(u"FreeSans");
        m_cachedName->setSizeIsSpatiumDependent(true);
    }

    if (!m_cachedClef) {
        m_cachedClef = engraving::Factory::createClef(seg, ACCESSIBILITY_DISABLED);
    }

    if (!m_cachedKeySig) {
        m_cachedKeySig = engraving::Factory::createKeySig(seg, ACCESSIBILITY_DISABLED);
        m_cachedKeySig->setHideNaturals(true);
    }

    if (!m_cachedTimeSig) {
        m_cachedTimeSig = engraving::Factory::createTimeSig(seg, ACCESSIBILITY_DISABLED);
    }

    if (!m_cachedBarLine) {
        m_cachedBarLine = engraving::Factory::createBarLine(seg, ACCESSIBILITY_DISABLED);
    }

    for (const engraving::EngravingItem* e : std::as_const(el)) {
        e->itemDiscovered = false;
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isStaffLines()) {
            const engraving::Staff* currentStaff = score->staff(e->staffIdx());
            const engraving::Instrument* instrument = currentStaff->part()->instrument(tick);

            // Find maximum width for the staff name
            const StaffNameList& staffNamesLong = instrument->longNames();
            String staffName = staffNamesLong.empty() ? String() : staffNamesLong.front().name();
            if (staffName.empty()) {
                const StaffNameList& staffNamesShort = instrument->shortNames();
                staffName = staffNamesShort.empty() ? String() : staffNamesShort.front().name();
            }

            m_cachedName->setParent(seg);
            m_cachedName->setXmlText(staffName);
            m_cachedName->setTrack(e->track());
            m_cachedName->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedName);
            m_cachedName->setPlainText(m_cachedName->plainText());
            m_cachedName->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedName);

            if (m_cachedName->width() > lineWidthName && !m_cachedName->xmlText().empty()) {
                lineWidthName = m_cachedName->width();
            }

            // Clef
            m_cachedClef->setParent(seg);
            engraving::ClefType currentClef = currentStaff->clef(tick);
            m_cachedClef->setClefType(currentClef);
            m_cachedClef->setTrack(e->track());
            m_cachedClef->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedClef);
            if (m_cachedClef->width() > widthClef) {
                widthClef = m_cachedClef->width();
            }

            // Key Signature
            m_cachedKeySig->setParent(seg);
            engraving::KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(tick);
            m_cachedKeySig->setKeySigEvent(currentKeySigEvent);
            m_cachedKeySig->setTrack(e->track());
            m_cachedKeySig->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedKeySig);
            if (m_cachedKeySig->width() > widthKeySig) {
                widthKeySig = m_cachedKeySig->width();
            }

            // Time Signature
            m_cachedTimeSig->setParent(seg);

            // Try to get local time signature, if not, get the current measure one
            engraving::TimeSig* currentTimeSig = currentStaff->timeSig(tick);
            if (currentTimeSig) {
                m_cachedTimeSig->setFrom(currentTimeSig);
            } else {
                m_cachedTimeSig->setSig(currentTimeSigFraction, TimeSigType::NORMAL);
            }
            m_cachedTimeSig->setTrack(e->track());
            m_cachedTimeSig->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedTimeSig);

            if (m_cachedTimeSig->width() > widthTimeSig) {
                widthTimeSig = m_cachedTimeSig->width();
            }
        }
    }

    leftMarginTotal = styleMM(engraving::Sid::clefLeftMargin);
    leftMarginTotal += styleMM(engraving::Sid::keysigLeftMargin);
    leftMarginTotal += styleMM(engraving::Sid::timesigLeftMargin);

    newWidth = widthClef + widthKeySig + widthTimeSig + leftMarginTotal + panelRightPadding;
    xPosMeasure -= offsetPanel;

    lineWidthName += spatium + styleMM(engraving::Sid::clefLeftMargin) + widthClef;
    if (newWidth < lineWidthName) {
        newWidth = lineWidthName;
        oldWidth = 0;
    }
    if (oldWidth == 0) {
        oldWidth = newWidth;
        m_width = newWidth;
    } else if (newWidth > 0) {
        if (newWidth == m_width) {
            oldWidth = m_width;
            m_width = newWidth;
        } else if (((xPosMeasure <= newWidth) && (xPosMeasure >= oldWidth))
                   || ((xPosMeasure >= newWidth) && (xPosMeasure <= oldWidth))) {
            m_width = xPosMeasure;
        } else if (((xPosMeasure + _measureWidth <= newWidth) && (xPosMeasure + _measureWidth >= oldWidth))
                   || ((xPosMeasure + _measureWidth >= newWidth) && (xPosMeasure + _measureWidth <= oldWidth))) {
            m_width = xPosMeasure + _measureWidth;
        } else {
            oldWidth = m_width;
            m_width = newWidth;
        }
    }

    m_rect = RectF(0, y, m_width, height);

    //====================

    painter.save();

    // Draw background rectangle
    PointF pos(offsetPanel, 0);

    painter.translate(pos);
    Pen pen;
    pen.setWidthF(0.0);
    pen.setStyle(PenStyle::NoPen);
    painter.setPen(pen);
    painter.setBrush(notationConfiguration()->foregroundColor());

    RectF bg(m_rect);
    bg.setWidth(widthClef + widthKeySig + widthTimeSig + leftMarginTotal + panelRightPadding);

    const QPixmap& wallpaper = notationConfiguration()->foregroundWallpaper();

    if (notationConfiguration()->foregroundUseColor() || wallpaper.isNull()) {
        painter.fillRect(bg, notationConfiguration()->foregroundColor());
    } else {
        painter.drawTiledPixmap(bg, wallpaper, bg.topLeft() - PointF(lrint(ctx.xOffset), lrint(ctx.yOffset)));
    }

    const Color color = engravingConfiguration()->invisibleColor();

    // Draw measure text number using cached text object
    const String text = String(u"#%1").arg(currentMeasure->no() + 1);
    m_cachedText->setXmlText(text);
    m_cachedText->setColor(color);
    m_cachedText->mutldata()->reset();
    m_cachedText->renderer()->layoutText1(m_cachedText);
    pos = PointF(styleMM(engraving::Sid::clefLeftMargin) + widthClef, y + m_cachedText->height());
    painter.translate(pos);
    m_cachedText->renderer()->drawItem(m_cachedText, &painter, opt);

    pos += PointF(offsetPanel, 0);
    painter.translate(-pos);

    // This second pass draws the elements spaced evenly using the width of the largest element
    for (const engraving::EngravingItem* e : std::as_const(el)) {
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isStaffLines()) {
            painter.save();
            const engraving::Staff* currentStaff = score->staff(e->staffIdx());
            const engraving::Instrument* instrument = currentStaff->part()->instrument(tick);

            pos = PointF(offsetPanel, e->pagePos().y());
            painter.translate(pos);

            // Draw staff lines
            engraving::StaffLines newStaffLines(*toStaffLines(e));
            newStaffLines.setParent(seg->measure());
            newStaffLines.setTrack(e->track());
            {
                LayoutContext cntx(newStaffLines.score());
                TLayout::layoutForWidth(&newStaffLines, bg.width(), cntx);
            }
            newStaffLines.setColor(color);
            scoreRender()->drawItem(&newStaffLines, &painter, opt);

            // Draw barline using cached object
            m_cachedBarLine->setParent(seg);
            m_cachedBarLine->setBarLineType(engraving::BarLineType::NORMAL);
            m_cachedBarLine->setTrack(e->track());
            m_cachedBarLine->setSpanStaff(currentStaff->barLineSpan());
            m_cachedBarLine->setSpanFrom(currentStaff->barLineFrom());
            m_cachedBarLine->setSpanTo(currentStaff->barLineTo());
            m_cachedBarLine->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedBarLine);
            m_cachedBarLine->setColor(color);
            m_cachedBarLine->renderer()->drawItem(m_cachedBarLine, &painter, opt);

            // Draw the current staff name using cached object
            const StaffNameList& staffNamesLong = instrument->longNames();
            String staffName = staffNamesLong.empty() ? String() : staffNamesLong.front().name();
            if (staffName.empty()) {
                const StaffNameList& staffNamesShort = instrument->shortNames();
                staffName = staffNamesShort.empty() ? String() : staffNamesShort.front().name();
            }

            m_cachedName->setParent(seg);
            m_cachedName->setXmlText(staffName);
            m_cachedName->setTrack(e->track());
            m_cachedName->setColor(color);
            m_cachedName->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedName);
            m_cachedName->setPlainText(m_cachedName->plainText());
            m_cachedName->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedName);

            if (currentStaff->part()->staff(0) == currentStaff) {
                const double spatium2 = score->style().spatium();
                pos = PointF(styleMM(engraving::Sid::clefLeftMargin) + widthClef, -spatium2 * 2);
                painter.translate(pos);
                m_cachedName->renderer()->drawItem(m_cachedName, &painter, opt);

                painter.translate(-pos);
            }

            double posX = 0.0;

            // Draw the current Clef using cached object
            m_cachedClef->setParent(seg);
            m_cachedClef->setClefType(currentStaff->clef(tick));
            m_cachedClef->setTrack(e->track());
            m_cachedClef->setColor(color);
            m_cachedClef->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedClef);
            posX += styleMM(engraving::Sid::clefLeftMargin);
            const PointF clefPos = PointF(posX, m_cachedClef->pos().y());
            painter.translate(clefPos);
            scoreRender()->drawItem(m_cachedClef, &painter, opt);
            painter.translate(-clefPos);
            posX += widthClef;

            // Draw the current KeySignature using cached object
            m_cachedKeySig->setParent(seg);
            m_cachedKeySig->setKeySigEvent(currentStaff->keySigEvent(tick));
            m_cachedKeySig->setTrack(e->track());
            m_cachedKeySig->setColor(color);
            m_cachedKeySig->mutldata()->reset();
            scoreRender()->layoutItem(m_cachedKeySig);
            posX += styleMM(engraving::Sid::keysigLeftMargin);
            const PointF ksPos = PointF(posX, 0.0);
            painter.translate(ksPos);
            scoreRender()->drawItem(m_cachedKeySig, &painter, opt);
            painter.translate(-ksPos);

            posX += widthKeySig + xPosTimeSig;

            // Draw the current TimeSignature using cached object
            m_cachedTimeSig->setParent(seg);

            // Try to get local time signature, if not, get the current measure one
            const engraving::TimeSig* currentTimeSig = currentStaff->timeSig(tick);
            if (currentTimeSig) {
                m_cachedTimeSig->setFrom(currentTimeSig);
                m_cachedTimeSig->setTrack(e->track());
                m_cachedTimeSig->setColor(color);
                m_cachedTimeSig->mutldata()->reset();
                scoreRender()->layoutItem(m_cachedTimeSig);
                posX += styleMM(engraving::Sid::timesigLeftMargin);
                const PointF tsPos = PointF(posX, 0.0);
                painter.translate(tsPos);
                scoreRender()->drawItem(m_cachedTimeSig, &painter, opt);
                painter.translate(-tsPos);
            }

            painter.restore();
        }
    }

    painter.restore();
}

double ContinuousPanel::styleMM(const mu::engraving::Sid styleId) const
{
    return score()->style().styleMM(styleId).val();
}

const mu::engraving::Score* ContinuousPanel::score() const
{
    return m_notation->elements()->msScore();
}
