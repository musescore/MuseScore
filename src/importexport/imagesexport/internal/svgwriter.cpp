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

#include "svgwriter.h"

#include "draw/painter.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafflines.h"
#include "engraving/dom/system.h"
#include "engraving/dom/repeatlist.h"

#include "svggenerator.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;

std::vector<INotationWriter::UnitType> SvgWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

mu::Ret SvgWriter::write(INotationPtr notation, QIODevice& destinationDevice, const Options& options)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    score->setPrinting(true); // don’t print page break symbols etc.

    mu::engraving::MScore::pdfPrinting = true;
    mu::engraving::MScore::svgPrinting = true;

    const std::vector<mu::engraving::Page*>& pages = score->pages();
    double pixelRationBackup = mu::engraving::MScore::pixelRatio;

    const size_t PAGE_NUMBER = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    if (PAGE_NUMBER >= pages.size()) {
        return false;
    }

    mu::engraving::Page* page = pages.at(PAGE_NUMBER);

    SvgGenerator printer;
    QString title(score->name());
    printer.setTitle(pages.size() > 1 ? QString("%1 (%2)").arg(title).arg(PAGE_NUMBER + 1) : title);
    printer.setOutputDevice(&destinationDevice);

    const int TRIM_MARGIN_SIZE = configuration()->trimMarginPixelSize();

    RectF pageRect = page->abbox();
    if (TRIM_MARGIN_SIZE >= 0) {
        pageRect = page->tbbox().adjusted(-TRIM_MARGIN_SIZE, -TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE);
    }

    qreal width = pageRect.width();
    qreal height = pageRect.height();
    printer.setSize(QSize(width, height));
    printer.setViewBox(QRectF(0, 0, width, height));

    mu::draw::Painter painter(&printer, "svgwriter");
    painter.setAntialiasing(true);
    if (TRIM_MARGIN_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    mu::engraving::MScore::pixelRatio = mu::engraving::DPI / printer.logicalDpiX();

    if (!options[OptionKey::TRANSPARENT_BACKGROUND].toBool()) {
        painter.fillRect(pageRect, mu::draw::Color::WHITE);
    }

    // 1st pass: StaffLines
    for (const mu::engraving::System* system : page->systems()) {
        size_t stavesCount = system->staves().size();

        for (size_t staffIndex = 0; staffIndex < stavesCount; ++staffIndex) {
            if (score->staff(staffIndex)->isLinesInvisible(mu::engraving::Fraction(0, 1)) || !score->staff(staffIndex)->show()) {
                continue; // ignore invisible staves
            }

            if (system->staves().empty() || !system->staff(staffIndex)->show()) {
                continue;
            }

            mu::engraving::Measure* firstMeasure = system->firstMeasure();
            if (!firstMeasure) { // only boxes, hence no staff lines
                continue;
            }

            // The goal here is to draw SVG staff lines more efficiently.
            // MuseScore draws staff lines by measure, but for SVG they can
            // generally be drawn once for each system. This makes a big
            // difference for scores that scroll horizontally on a single
            // page. But there are exceptions to this rule:
            //
            //   ~ One (or more) invisible measure(s) in a system/staff ~
            //   ~ One (or more) elements of type HBOX or VBOX          ~
            //
            // In these cases the SVG staff lines for the system/staff
            // are drawn by measure.
            //
            bool byMeasure = false;
            for (mu::engraving::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                if (!measure->isMeasure() || !mu::engraving::toMeasure(measure)->visible(staffIndex)) {
                    byMeasure = true;
                    break;
                }
            }

            if (byMeasure) {     // Draw visible staff lines by measure
                for (mu::engraving::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                    if (measure->isMeasure() && mu::engraving::toMeasure(measure)->visible(staffIndex)) {
                        mu::engraving::StaffLines* sl = mu::engraving::toMeasure(measure)->staffLines(static_cast<int>(staffIndex));
                        printer.setElement(sl);
                        scoreRenderer()->paintItem(painter, sl);
                    }
                }
            } else {   // Draw staff lines once per system
                mu::engraving::StaffLines* firstSL = system->firstMeasure()->staffLines(static_cast<int>(staffIndex))->clone();
                mu::engraving::StaffLines* lastSL =  system->lastMeasure()->staffLines(static_cast<int>(staffIndex));

                qreal lastX =  lastSL->ldata()->bbox().right()
                              + lastSL->pagePos().x()
                              - firstSL->pagePos().x();
                std::vector<mu::LineF> lines = firstSL->lines();
                for (size_t l = 0, c = lines.size(); l < c; l++) {
                    lines[l].setP2(mu::PointF(lastX, lines[l].p2().y()));
                }
                firstSL->setLines(lines);

                printer.setElement(firstSL);
                scoreRenderer()->paintItem(painter, firstSL);
            }
        }
    }

    BeatsColors beatsColors = parseBeatsColors(options.value(OptionKey::BEATS_COLORS, Val()).toQVariant());

    // 2nd pass: Set color for elements on beats
    int beatIndex = 0;
    for (const mu::engraving::RepeatSegment* repeatSegment : score->repeatList()) {
        for (const mu::engraving::Measure* measure : repeatSegment->measureList()) {
            for (mu::engraving::Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType()) {
                    continue;
                }

                if (beatsColors.contains(beatIndex)) {
                    for (EngravingItem* element : segment->elist()) {
                        if (!element) {
                            continue;
                        }

                        if (element->isChord()) {
                            for (Note* note : toChord(element)->notes()) {
                                note->setColor(beatsColors[beatIndex]);
                            }
                        } else if (element->isChordRest()) {
                            element->setColor(beatsColors[beatIndex]);
                        }
                    }
                }

                beatIndex++;
            }
        }
    }

    // 3rd pass: the rest of the elements
    std::vector<mu::engraving::EngravingItem*> elements = page->elements();
    std::sort(elements.begin(), elements.end(), mu::engraving::elementLessThan);

    for (const mu::engraving::EngravingItem* element : elements) {
        // Always exclude invisible elements
        if (!element->visible()) {
            continue;
        }

        mu::engraving::ElementType type = element->type();
        switch (type) { // In future sub-type code, this switch() grows, and eType gets used
        case mu::engraving::ElementType::STAFF_LINES: // Handled in the 1st pass above
            continue; // Exclude from 2nd pass
            break;
        default:
            break;
        }

        // Set the EngravingItem pointer inside SvgGenerator/SvgPaintEngine
        printer.setElement(element);

        // Paint it
        scoreRenderer()->paintItem(painter, element);
    }

    painter.endDraw(); // Writes MuseScore SVG file to disk, finally

    // Clean up and return
    mu::engraving::MScore::pixelRatio = pixelRationBackup;
    score->setPrinting(false);
    mu::engraving::MScore::pdfPrinting = false;
    mu::engraving::MScore::svgPrinting = false;

    return true;
}

SvgWriter::BeatsColors SvgWriter::parseBeatsColors(const QVariant& obj) const
{
    QVariantMap map = obj.toMap();
    BeatsColors result;

    for (const QString& beatNumber : map.keys()) {
        result[beatNumber.toInt()] = map[beatNumber].value<QColor>();
    }

    return result;
}
