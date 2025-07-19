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

#include "svgwriter.h"

#include <QBuffer>

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
using namespace muse;
using namespace muse::io;

std::vector<INotationWriter::UnitType> SvgWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

Ret SvgWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options& options)
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

    const size_t PAGE_NUMBER = muse::value(options, OptionKey::PAGE_NUMBER, Val(0)).toInt();
    if (PAGE_NUMBER >= pages.size()) {
        return false;
    }

    mu::engraving::Page* page = pages.at(PAGE_NUMBER);

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    SvgGenerator printer;
    QString title(score->name());
    printer.setTitle(pages.size() > 1 ? QString("%1 (%2)").arg(title).arg(PAGE_NUMBER + 1) : title);
    printer.setOutputDevice(&buf);

    printer.setReplaceClipPathWithMask(configuration()->exportSvgWithIllustratorCompat());

    const int TRIM_MARGIN_SIZE = configuration()->trimMarginPixelSize();

    RectF pageRect = page->pageBoundingRect();
    if (TRIM_MARGIN_SIZE >= 0) {
        pageRect = page->tbbox().adjusted(-TRIM_MARGIN_SIZE, -TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE);
    }

    qreal width = pageRect.width();
    qreal height = pageRect.height();
    printer.setSize(QSize(width, height));
    printer.setViewBox(QRectF(0, 0, width, height));

    muse::draw::Painter painter(&printer, "svgwriter");
    painter.setAntialiasing(true);
    if (TRIM_MARGIN_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    mu::engraving::MScore::pixelRatio = mu::engraving::DPI / printer.logicalDpiX();

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
                                                    Val(configuration()->exportSvgWithTransparentBackground())).toBool();
    if (!TRANSPARENT_BACKGROUND) {
        painter.fillRect(pageRect, muse::draw::Color::WHITE);
    }

    // 1st pass: StaffLines
    for (const mu::engraving::System* system : page->systems()) {
        size_t stavesCount = system->staves().size();

        for (size_t staffIndex = 0; staffIndex < stavesCount; ++staffIndex) {
            if (!score->staff(staffIndex)->show()) {
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
            // page.

            mu::engraving::StaffLines* concatenatedSL = nullptr;
            mu::engraving::Shape concatenatedShape;
            mu::engraving::Shape concatenatedMask;
            StaffType* prevStaffType = nullptr;
            for (mu::engraving::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                if (!measure->isMeasure()) {
                    if (concatenatedSL != nullptr) {
                        printer.setElement(concatenatedSL);
                        scoreRenderer()->paintItem(painter, concatenatedSL);
                        concatenatedSL = nullptr;
                        prevStaffType = nullptr;
                    }
                    continue;
                }

                Measure* m = mu::engraving::toMeasure(measure);
                mu::engraving::StaffLines* sl = m->staffLines(static_cast<int>(staffIndex));

                if ((!m->visible(staffIndex) && !m->isCutawayClef(staffIndex)) || !sl->visible()
                    || (score->staff(staffIndex)->staffType(m->tick()) != prevStaffType)) {
                    if (concatenatedSL != nullptr) {
                        printer.setElement(concatenatedSL);
                        scoreRenderer()->paintItem(painter, concatenatedSL);
                        concatenatedSL = nullptr;
                        prevStaffType = nullptr;
                    }
                }

                if (concatenatedSL == nullptr) {
                    if ((m->visible(staffIndex) || m->isCutawayClef(staffIndex)) && sl->visible()) {
                        concatenatedSL = sl->clone();
                        concatenatedShape.add(sl->ldata()->shape());
                        concatenatedMask.add(sl->ldata()->mask());
                        prevStaffType = score->staff(staffIndex)->staffType(m->tick());
                    }
                } else {
                    qreal lastX = sl->ldata()->bbox().right()
                                  + sl->pagePos().x()
                                  - concatenatedSL->pagePos().x();
                    std::vector<muse::LineF> lines = concatenatedSL->lines();
                    for (size_t l = 0, c = lines.size(); l < c; l++) {
                        lines[l].setP2(muse::PointF(lastX, lines[l].p2().y()));
                    }
                    concatenatedSL->setLines(lines);
                    concatenatedShape.add(sl->ldata()->shape().translated(sl->pagePos() - concatenatedSL->pagePos()));
                    concatenatedMask.add(sl->ldata()->mask().translated(sl->pagePos() - concatenatedSL->pagePos()));
                }
            }

            if (concatenatedSL != nullptr) {
                concatenatedSL->mutldata()->setShape(concatenatedShape);
                concatenatedSL->mutldata()->setMask(concatenatedMask);
                printer.setElement(concatenatedSL);
                scoreRenderer()->paintItem(painter, concatenatedSL);
                concatenatedSL = nullptr;
                prevStaffType = nullptr;
            }
        }
    }

    BeatsColors beatsColors = parseBeatsColors(muse::value(options, OptionKey::BEATS_COLORS, Val()).toQVariant());

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

    painter.endDraw();

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

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
