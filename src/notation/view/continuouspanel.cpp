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

#include "containers.h"

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
#include "engraving/rendering/score/tlayout.h"

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
    delete m_cachedMeasureNumberText;
    m_cachedMeasureNumberText = nullptr;

    muse::DeleteAll(m_cachedStaffNameTexts);
    m_cachedStaffNameTexts.clear();

    muse::DeleteAll(m_cachedClefs);
    m_cachedClefs.clear();

    muse::DeleteAll(m_cachedKeySigs);
    m_cachedKeySigs.clear();

    muse::DeleteAll(m_cachedTimeSigs);
    m_cachedTimeSigs.clear();

    muse::DeleteAll(m_cachedBarLines);
    m_cachedBarLines.clear();
}

void ContinuousPanel::ensureCacheSize(size_t staffCount)
{
    // Resize vectors if needed and create objects
    if (m_cachedStaffNameTexts.size() < staffCount) {
        m_cachedStaffNameTexts.resize(staffCount, nullptr);
    }
    if (m_cachedClefs.size() < staffCount) {
        m_cachedClefs.resize(staffCount, nullptr);
    }
    if (m_cachedKeySigs.size() < staffCount) {
        m_cachedKeySigs.resize(staffCount, nullptr);
    }
    if (m_cachedTimeSigs.size() < staffCount) {
        m_cachedTimeSigs.resize(staffCount, nullptr);
    }
    if (m_cachedBarLines.size() < staffCount) {
        m_cachedBarLines.resize(staffCount, nullptr);
    }
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

qreal ContinuousPanel::width() const
{
    return m_width;
}

void ContinuousPanel::paint(Painter& painter, const NotationViewContext& ctx, const engraving::rendering::PaintOptions& opt)
{
    TRACEFUNC;

    if (!m_notation) {
        return;
    }

    const engraving::Score* score = m_notation->elements()->msScore();

    double offsetPanel = -ctx.xOffset / ctx.scaling;
    double y = 0;
    double oldWidth = 0;          // The last final panel width
    double newWidth = 0;          // New panel width
    double height = 0;
    double leftMarginTotal = 0;   // Sum of all elements left margin
    double panelRightPadding = 5;    // Extra space for the panel after last element

    const engraving::Measure* measure = score->firstMeasure()->coveringMMRestOrThis();
    if (!measure) {
        return;
    }

    const engraving::System* system = measure->system();
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

    // Ensure cache size matches staff count
    ensureCacheSize(staffCount);

    if (!m_cachedMeasureNumberText) {
        m_cachedMeasureNumberText = engraving::Factory::createText(seg, mu::engraving::TextStyleType::DEFAULT, ACCESSIBILITY_DISABLED);
        m_cachedMeasureNumberText->setFlag(engraving::ElementFlag::MOVABLE, false);
        m_cachedMeasureNumberText->setFamily(u"FreeSans");
        m_cachedMeasureNumberText->setSizeIsSpatiumDependent(true);
    }

    // Track which staff index we're processing
    size_t staffIdx = 0;

    for (const engraving::EngravingItem* e : std::as_const(el)) {
        e->itemDiscovered = false;
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isStaffLines()) {
            staffIdx = e->staffIdx();
            const engraving::Staff* currentStaff = score->staff(staffIdx);
            const engraving::Instrument* instrument = currentStaff->part()->instrument(tick);

            // Staff name
            const StaffNameList& staffNamesLong = instrument->longNames();
            String staffName = staffNamesLong.empty() ? String() : staffNamesLong.front().name();
            if (staffName.empty()) {
                const StaffNameList& staffNamesShort = instrument->shortNames();
                staffName = staffNamesShort.empty() ? String() : staffNamesShort.front().name();
            }

            engraving::Text*& nameText = m_cachedStaffNameTexts[staffIdx];
            if (!nameText) {
                nameText = engraving::Factory::createText(seg, mu::engraving::TextStyleType::DEFAULT, ACCESSIBILITY_DISABLED);
                nameText->setFlag(engraving::ElementFlag::MOVABLE, false);
                nameText->setFamily(u"FreeSans");
                nameText->setSizeIsSpatiumDependent(true);
            }

            nameText->setParent(seg);
            nameText->setXmlText(staffName);
            nameText->setTrack(e->track());
            nameText->mutldata()->reset();
            scoreRender()->layoutItem(nameText);
            nameText->setPlainText(nameText->plainText());
            nameText->mutldata()->reset();
            scoreRender()->layoutItem(nameText);

            if (nameText->width() > lineWidthName && !nameText->xmlText().empty()) {
                lineWidthName = nameText->width();
            }

            // Clef
            engraving::Clef*& clef = m_cachedClefs[staffIdx];
            if (!clef) {
                clef = engraving::Factory::createClef(seg, ACCESSIBILITY_DISABLED);
                clef->setTrack(e->track());
            }

            clef->setParent(seg);
            engraving::ClefType currentClef = currentStaff->clef(tick);
            clef->setClefType(currentClef);
            clef->mutldata()->reset();
            scoreRender()->layoutItem(clef);
            if (clef->width() > widthClef) {
                widthClef = clef->width();
            }

            // Key Signature
            engraving::KeySig*& keySig = m_cachedKeySigs[staffIdx];
            if (!keySig) {
                keySig = engraving::Factory::createKeySig(seg, ACCESSIBILITY_DISABLED);
                keySig->setHideNaturals(true);
                keySig->setTrack(e->track());
            }

            keySig->setParent(seg);
            engraving::KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(tick);
            keySig->setKeySigEvent(currentKeySigEvent);
            keySig->mutldata()->reset();
            scoreRender()->layoutItem(keySig);
            if (keySig->width() > widthKeySig) {
                widthKeySig = keySig->width();
            }

            // Time Signature
            engraving::TimeSig*& timeSig = m_cachedTimeSigs[staffIdx];
            if (!timeSig) {
                timeSig = engraving::Factory::createTimeSig(seg, ACCESSIBILITY_DISABLED);
                timeSig->setTrack(e->track());
            }
            timeSig->setParent(seg);

            // Try to get local time signature, if not, get the current measure one
            engraving::TimeSig* currentTimeSig = currentStaff->timeSig(tick);
            if (currentTimeSig) {
                timeSig->setFrom(currentTimeSig);
            } else {
                timeSig->setSig(currentTimeSigFraction, TimeSigType::NORMAL);
            }
            timeSig->mutldata()->reset();
            scoreRender()->layoutItem(timeSig);

            if (timeSig->width() > widthTimeSig) {
                widthTimeSig = timeSig->width();
            }

            // Bar line - layout and store
            engraving::BarLine*& barLine = m_cachedBarLines[staffIdx];
            if (!barLine) {
                barLine = engraving::Factory::createBarLine(seg, ACCESSIBILITY_DISABLED);
                barLine->setBarLineType(engraving::BarLineType::NORMAL);
                barLine->setTrack(e->track());
            }

            barLine->setParent(seg);
            barLine->setSpanStaff(currentStaff->barLineSpan());
            barLine->setSpanFrom(currentStaff->barLineFrom());
            barLine->setSpanTo(currentStaff->barLineTo());
            barLine->mutldata()->reset();
            scoreRender()->layoutItem(barLine);
        }
    }

    const double clefLeftMargin = score->style().styleMM(engraving::Sid::clefLeftMargin);
    const double keySigLeftMargin = score->style().styleMM(engraving::Sid::keysigLeftMargin);
    const double timeSigLeftMargin = score->style().styleMM(engraving::Sid::timesigLeftMargin);

    leftMarginTotal = clefLeftMargin;
    leftMarginTotal += keySigLeftMargin;
    leftMarginTotal += timeSigLeftMargin;

    newWidth = widthClef + widthKeySig + widthTimeSig + leftMarginTotal + panelRightPadding;
    xPosMeasure -= offsetPanel;

    lineWidthName += spatium + clefLeftMargin + widthClef;
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
        painter.drawTiledPixmap(bg, wallpaper, bg.topLeft() - PointF(ctx.xOffset, ctx.yOffset));
    }

    const Color color = engravingConfiguration()->invisibleColor();

    // Draw measure number
    m_cachedMeasureNumberText->setXmlText(String(u"#%1").arg(currentMeasure->no() + 1));
    m_cachedMeasureNumberText->setColor(color);
    m_cachedMeasureNumberText->mutldata()->reset();
    m_cachedMeasureNumberText->renderer()->layoutText1(m_cachedMeasureNumberText);
    pos = PointF(clefLeftMargin + widthClef, y + m_cachedMeasureNumberText->height());
    painter.translate(pos);
    m_cachedMeasureNumberText->renderer()->drawItem(m_cachedMeasureNumberText, &painter, opt);

    pos += PointF(offsetPanel, 0);
    painter.translate(-pos);

    // This second pass draws the elements using already laid-out cached objects
    for (const engraving::EngravingItem* e : std::as_const(el)) {
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isStaffLines()) {
            painter.save();
            const size_t staffIdx = e->staffIdx();
            const engraving::Staff* currentStaff = score->staff(staffIdx);

            pos = PointF(offsetPanel, e->pagePos().y());
            painter.translate(pos);

            // Staff lines
            engraving::StaffLines newStaffLines(*toStaffLines(e));
            newStaffLines.setParent(seg->measure());
            newStaffLines.setTrack(e->track());
            {
                LayoutContext cntx(newStaffLines.score());
                TLayout::layoutForWidth(&newStaffLines, bg.width(), cntx);
            }
            newStaffLines.setColor(color);
            scoreRender()->drawItem(&newStaffLines, &painter, opt);

            // Barline
            engraving::BarLine*& barLine = m_cachedBarLines[staffIdx];
            barLine->setColor(color);
            barLine->renderer()->drawItem(barLine, &painter, opt);

            // Staff name
            if (currentStaff->part()->staff(0) == currentStaff) {
                const double spatium2 = score->style().spatium();
                pos = PointF(clefLeftMargin + widthClef, -spatium2 * 2);
                painter.translate(pos);
                m_cachedStaffNameTexts[staffIdx]->setColor(color);
                m_cachedStaffNameTexts[staffIdx]->renderer()->drawItem(m_cachedStaffNameTexts[staffIdx], &painter, opt);

                painter.translate(-pos);
            }

            double posX = 0.0;

            // Clef
            engraving::Clef*& clef = m_cachedClefs[staffIdx];
            clef->setColor(color);
            posX += clefLeftMargin;
            const PointF clefPos = PointF(posX, clef->pos().y());
            painter.translate(clefPos);
            scoreRender()->drawItem(clef, &painter, opt);
            painter.translate(-clefPos);
            posX += widthClef;

            // Key signature
            engraving::KeySig*& keySig = m_cachedKeySigs[staffIdx];
            keySig->setColor(color);
            posX += keySigLeftMargin;
            const PointF ksPos = PointF(posX, 0.0);
            painter.translate(ksPos);
            scoreRender()->drawItem(keySig, &painter, opt);
            painter.translate(-ksPos);

            posX += widthKeySig + xPosTimeSig;

            // Time signature
            engraving::TimeSig*& timeSig = m_cachedTimeSigs[staffIdx];
            timeSig->setColor(color);
            posX += timeSigLeftMargin;
            const PointF tsPos = PointF(posX, 0.0);
            painter.translate(tsPos);
            scoreRender()->drawItem(timeSig, &painter, opt);
            painter.translate(-tsPos);

            painter.restore();
        }
    }

    painter.restore();
}
