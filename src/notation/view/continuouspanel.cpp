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
#include "engraving/dom/rest.h"
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

void ContinuousPanel::setNotation(INotationPtr notation)
{
    m_notation = notation;
}

//! NOTE: Copied from MU3
void ContinuousPanel::paint(Painter& painter, const NotationViewContext& ctx)
{
    TRACEFUNC;

    const mu::engraving::Score* score = this->score();
    if (!score) {
        return;
    }

    qreal offsetPanel = -ctx.xOffset / ctx.scaling;
    qreal y = 0;
    qreal oldWidth = 0;          // The last final panel width
    qreal newWidth = 0;          // New panel width
    qreal height = 0;
    qreal leftMarginTotal = 0;   // Sum of all elements left margin
    qreal panelRightPadding = 5;    // Extra space for the panel after last element

    mu::engraving::Measure* measure = score->firstMeasure();
    if (!measure) {
        return;
    }

    if (measure->mmRest()) {
        measure = measure->mmRest();
    }

    mu::engraving::System* system = measure->system();
    if (!system) {
        return;
    }

    mu::engraving::Segment* s = measure->first();
    double spatium = score->style().spatium();
    if (m_width <= 0) {
        m_width = s->x();
    }

    //
    // Set panel height for whole system
    //
    height = 6 * spatium;
    y = system->staffYpage(0) + system->page()->pos().y();
    double y2 = 0.0;
    size_t staffCount = score->nstaves();
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
    bool showInvisible = score->isShowInvisible();
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

    qreal canvasPosX = ctx.fromLogical(currentMeasure->canvasPos()).x();
    if (currentMeasure == score->firstMeasure() && canvasPosX > 0) {
        return;
    }

    qreal xPosMeasure = currentMeasure->canvasX();
    qreal _measureWidth = currentMeasure->width();
    int tick = currentMeasure->tick().ticks();
    mu::engraving::Fraction currentTimeSigFraction = currentMeasure->timesig();

    //---------------------------------------------------------
    //   findElementWidths
    //      determines the max width for each element types
    //---------------------------------------------------------

    // The first pass serves to get the maximum width for each elements

    qreal lineWidthName = 0;
    qreal widthClef    = 0;
    qreal widthKeySig  = 0;
    qreal widthTimeSig = 0;
    qreal xPosTimeSig  = 0;

    for (const mu::engraving::EngravingItem* e : std::as_const(el)) {
        e->itemDiscovered = false;
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isRest() && toRest(e)->isGap()) {
            continue;
        }

        if (e->isStaffLines()) {
            mu::engraving::Staff* currentStaff = score->staff(e->staffIdx());
            mu::engraving::Segment* parent = score->tick2segment(Fraction::fromTicks(tick));

            // Find maximum width for the staff name
            const std::list<mu::engraving::StaffName>& staffNamesLong
                = currentStaff->part()->instrument(mu::engraving::Fraction::fromTicks(tick))->longNames();
            QString staffName = staffNamesLong.empty() ? u" " : staffNamesLong.front().name();
            if (staffName == "") {
                const std::list<mu::engraving::StaffName>& staffNamesShort
                    = currentStaff->part()->instrument(mu::engraving::Fraction::fromTicks(tick))->shortNames();
                staffName = staffNamesShort.empty() ? u"" : staffNamesShort.front().name();
            }

            mu::engraving::Text* newName = engraving::Factory::createText(parent, mu::engraving::TextStyleType::DEFAULT,
                                                                          ACCESSIBILITY_DISABLED);
            newName->setParent(parent);
            newName->setXmlText(staffName);
            newName->setTrack(e->track());
            newName->setFamily(u"FreeSans");
            newName->setSizeIsSpatiumDependent(true);
            scoreRender()->layoutItem(newName);
            newName->setPlainText(newName->plainText());
            scoreRender()->layoutItem(newName);

            // Find maximum width for the current Clef
            mu::engraving::Clef* newClef = engraving::Factory::createClef(parent, ACCESSIBILITY_DISABLED);
            newClef->setParent(parent);
            mu::engraving::ClefType currentClef = currentStaff->clef(mu::engraving::Fraction::fromTicks(tick));
            newClef->setClefType(currentClef);
            newClef->setTrack(e->track());
            scoreRender()->layoutItem(newClef);
            if (newClef->width() > widthClef) {
                widthClef = newClef->width();
            }

            // Find maximum width for the current KeySignature
            mu::engraving::KeySig* newKs = engraving::Factory::createKeySig(parent, ACCESSIBILITY_DISABLED);
            newKs->setParent(parent);
            mu::engraving::KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(mu::engraving::Fraction::fromTicks(tick));
            newKs->setKeySigEvent(currentKeySigEvent);
            // The Parent and the Track must be set to have the key signature layout adjusted to different clefs
            // This also adds naturals to the key signature (if set in the score style)
            newKs->setTrack(e->track());
            newKs->setHideNaturals(true);
            scoreRender()->layoutItem(newKs);
            if (newKs->width() > widthKeySig) {
                widthKeySig = newKs->width();
            }

            // Find maximum width for the current TimeSignature
            mu::engraving::TimeSig* newTs = engraving::Factory::createTimeSig(parent, ACCESSIBILITY_DISABLED);
            newTs->setParent(parent);

            // Try to get local time signature, if not, get the current measure one
            mu::engraving::TimeSig* currentTimeSig = currentStaff->timeSig(mu::engraving::Fraction::fromTicks(tick));
            if (currentTimeSig) {
                newTs->setFrom(currentTimeSig);
            } else {
                newTs->setSig(Fraction(currentTimeSigFraction.numerator(), currentTimeSigFraction.denominator()), TimeSigType::NORMAL);
            }
            newTs->setTrack(e->track());
            scoreRender()->layoutItem(newTs);

            if ((newName->width() > lineWidthName) && (newName->xmlText() != "")) {
                lineWidthName = newName->width();
            }

            if (newTs->width() > widthTimeSig) {
                widthTimeSig = newTs->width();
            }

            delete newClef;
            delete newName;
            delete newKs;
            delete newTs;
        }
    }

    leftMarginTotal = styleMM(mu::engraving::Sid::clefLeftMargin);
    leftMarginTotal += styleMM(mu::engraving::Sid::keysigLeftMargin);
    leftMarginTotal += styleMM(mu::engraving::Sid::timesigLeftMargin);

    newWidth = widthClef + widthKeySig + widthTimeSig + leftMarginTotal + panelRightPadding;
    xPosMeasure -= offsetPanel;

    lineWidthName += score->style().spatium() + styleMM(mu::engraving::Sid::clefLeftMargin) + widthClef;
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

    // Draw colored rectangle
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

    Color color = engravingConfiguration()->invisibleColor();

    // Draw measure text number
    // TODO: simplify (no Text element)
    QString text = QString("#%1").arg(currentMeasure->no() + 1);
    mu::engraving::Text* newElement = engraving::Factory::createText(
        score->dummy(), mu::engraving::TextStyleType::DEFAULT, ACCESSIBILITY_DISABLED);
    newElement->setFlag(mu::engraving::ElementFlag::MOVABLE, false);
    newElement->setXmlText(text);
    newElement->setFamily(u"FreeSans");
    newElement->setSizeIsSpatiumDependent(true);
    newElement->setColor(color);
    newElement->renderer()->layoutText1(newElement);
    pos = PointF(styleMM(mu::engraving::Sid::clefLeftMargin) + widthClef, y + newElement->height());
    painter.translate(pos);
    newElement->renderer()->drawItem(newElement, &painter);

    pos += PointF(offsetPanel, 0);
    painter.translate(-pos);
    delete newElement;

    // This second pass draws the elements spaced evenly using the width of the largest element
    for (const mu::engraving::EngravingItem* e : std::as_const(el)) {
        if (!e->visible() && !showInvisible) {
            continue;
        }

        if (e->isRest() && toRest(e)->isGap()) {
            continue;
        }

        if (e->isStaffLines()) {
            painter.save();
            mu::engraving::Staff* currentStaff = score->staff(e->staffIdx());
            mu::engraving::Segment* parent = score->tick2segmentMM(Fraction::fromTicks(tick));

            pos = PointF(offsetPanel, e->pagePos().y());
            painter.translate(pos);

            // Draw staff lines
            mu::engraving::StaffLines newStaffLines(*toStaffLines(e));
            newStaffLines.setParent(parent->measure());
            newStaffLines.setTrack(e->track());
            {
                LayoutContext cntx(newStaffLines.score());
                TLayout::layoutForWidth(&newStaffLines, bg.width(), cntx);
            }
            newStaffLines.setColor(color);
            scoreRender()->drawItem(&newStaffLines, &painter);

            // Draw barline
            mu::engraving::BarLine* barLine = engraving::Factory::createBarLine(parent, ACCESSIBILITY_DISABLED);
            barLine->setParent(parent);
            barLine->setBarLineType(mu::engraving::BarLineType::NORMAL);
            barLine->setTrack(e->track());
            barLine->setSpanStaff(currentStaff->barLineSpan());
            barLine->setSpanFrom(currentStaff->barLineFrom());
            barLine->setSpanTo(currentStaff->barLineTo());
            scoreRender()->layoutItem(barLine);
            barLine->setColor(color);
            barLine->renderer()->drawItem(barLine, &painter);

            // Draw the current staff name
            const std::list<mu::engraving::StaffName>& staffNamesLong
                = currentStaff->part()->instrument(mu::engraving::Fraction::fromTicks(tick))->longNames();
            QString staffName = staffNamesLong.empty() ? u" " : staffNamesLong.front().name();
            if (staffName == "") {
                const std::list<mu::engraving::StaffName>& staffNamesShort
                    = currentStaff->part()->instrument(mu::engraving::Fraction::fromTicks(tick))->shortNames();
                staffName = staffNamesShort.empty() ? u"" : staffNamesShort.front().name();
            }

            mu::engraving::Text* newName = engraving::Factory::createText(parent, mu::engraving::TextStyleType::DEFAULT,
                                                                          ACCESSIBILITY_DISABLED);
            newName->setParent(parent);
            newName->setXmlText(staffName);
            newName->setTrack(e->track());
            newName->setColor(color);
            newName->setFamily(u"FreeSans");
            newName->setSizeIsSpatiumDependent(true);
            scoreRender()->layoutItem(newName);
            newName->setPlainText(newName->plainText());
            scoreRender()->layoutItem(newName);

            if (currentStaff->part()->staff(0) == currentStaff) {
                const double spatium2 = score->style().spatium();
                pos = PointF(styleMM(mu::engraving::Sid::clefLeftMargin) + widthClef, -spatium2 * 2);
                painter.translate(pos);
                newName->renderer()->drawItem(newName, &painter);

                painter.translate(-pos);
            }
            delete newName;

            qreal posX = 0.0;

            // Draw the current Clef
            mu::engraving::Clef* clef = engraving::Factory::createClef(parent, ACCESSIBILITY_DISABLED);
            clef->setParent(parent);
            clef->setClefType(currentStaff->clef(mu::engraving::Fraction::fromTicks(tick)));
            clef->setTrack(e->track());
            clef->setColor(color);
            scoreRender()->layoutItem(clef);
            posX += styleMM(mu::engraving::Sid::clefLeftMargin);
            clef->drawAt(&painter, PointF(posX, clef->pos().y()));
            posX += widthClef;

            // Draw the current KeySignature
            mu::engraving::KeySig* newKs = engraving::Factory::createKeySig(parent, ACCESSIBILITY_DISABLED);
            newKs->setParent(parent);
            newKs->setKeySigEvent(currentStaff->keySigEvent(mu::engraving::Fraction::fromTicks(tick)));

            // The Parent and the track must be set to have the key signature layout adjusted to different clefs
            // This also adds naturals to the key signature (if set in the score style)
            newKs->setTrack(e->track());
            newKs->setColor(color);
            newKs->setHideNaturals(true);
            scoreRender()->layoutItem(newKs);
            posX += styleMM(mu::engraving::Sid::keysigLeftMargin);
            newKs->drawAt(&painter, PointF(posX, 0.0));

            posX += widthKeySig + xPosTimeSig;

            // Draw the current TimeSignature
            mu::engraving::TimeSig* newTs = engraving::Factory::createTimeSig(parent, ACCESSIBILITY_DISABLED);
            newTs->setParent(parent);

            // Try to get local time signature, if not, get the current measure one
            mu::engraving::TimeSig* currentTimeSig = currentStaff->timeSig(mu::engraving::Fraction::fromTicks(tick));
            if (currentTimeSig) {
                newTs->setFrom(currentTimeSig);
                newTs->setTrack(e->track());
                newTs->setColor(color);
                scoreRender()->layoutItem(newTs);
                posX += styleMM(mu::engraving::Sid::timesigLeftMargin);
                newTs->drawAt(&painter, PointF(posX, 0.0));
            }

            delete newKs;
            delete newTs;
            delete clef;
            delete barLine;

            painter.restore();
        }
    }

    painter.restore();
}

qreal ContinuousPanel::styleMM(const mu::engraving::Sid styleId) const
{
    return score()->style().styleMM(styleId).val();
}

const mu::engraving::Score* ContinuousPanel::score() const
{
    return m_notation->elements()->msScore();
}
