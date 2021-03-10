//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "svgwriter.h"

#include "log.h"

#include "svggenerator.h"

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/stafflines.h"

#include "libmscore/draw/qpainterprovider.h"

using namespace mu::iex::imagesexport;
using namespace mu::system;

mu::Ret SvgWriter::write(const notation::INotationPtr notation, IODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    score->setPrinting(true); // don’t print page break symbols etc.

    Ms::MScore::pdfPrinting = true;
    Ms::MScore::svgPrinting = true;

    const QList<Ms::Page*>& pages = score->pages();
    double pixelRationBackup = Ms::MScore::pixelRatio;

    const int PAGE_NUMBER = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    if (PAGE_NUMBER < 0 || PAGE_NUMBER >= pages.size()) {
        return false;
    }

    Ms::Page* page = pages.at(PAGE_NUMBER);

    SvgGenerator printer;
    QString title(score->title());
    printer.setTitle(pages.size() > 1 ? QString("%1 (%2)").arg(title).arg(PAGE_NUMBER + 1) : title);
    printer.setOutputDevice(&destinationDevice);

    const int TRIM_MARGINS_SIZE = options.value(OptionKey::TRIM_MARGINS_SIZE, Val(0)).toInt();

    QRectF pageRect = page->abbox();
    if (TRIM_MARGINS_SIZE >= 0) {
        QMarginsF margins(TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE);
        pageRect = page->tbbox() + margins;
    }

    qreal width = pageRect.width();
    qreal height = pageRect.height();
    printer.setSize(QSize(width, height));
    printer.setViewBox(QRectF(0, 0, width, height));

    mu::draw::Painter painter(&printer, "svgwriter");
    painter.setAntialiasing(true);
    if (TRIM_MARGINS_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    Ms::MScore::pixelRatio = Ms::DPI / printer.logicalDpiX();

    if (!options[OptionKey::TRANSPARENT_BACKGROUND].toBool()) {
        painter.fillRect(pageRect, Qt::white);
    }

    // 1st pass: StaffLines
    for (const Ms::System* system : page->systems()) {
        int stavesCount = system->staves()->size();

        for (int staffIndex = 0; staffIndex < stavesCount; ++staffIndex) {
            if (score->staff(staffIndex)->invisible(Ms::Fraction(0,1)) || !score->staff(staffIndex)->show()) {
                continue; // ignore invisible staves
            }

            if (system->staves()->isEmpty() || !system->staff(staffIndex)->show()) {
                continue;
            }

            Ms::Measure* firstMeasure = system->firstMeasure();
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
            for (Ms::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                if (!measure->isMeasure() || !Ms::toMeasure(measure)->visible(staffIndex)) {
                    byMeasure = true;
                    break;
                }
            }

            if (byMeasure) {     // Draw visible staff lines by measure
                for (Ms::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                    if (measure->isMeasure() && Ms::toMeasure(measure)->visible(staffIndex)) {
                        Ms::StaffLines* sl = Ms::toMeasure(measure)->staffLines(staffIndex);
                        printer.setElement(sl);
                        Ms::paintElement(painter, sl);
                    }
                }
            } else {   // Draw staff lines once per system
                Ms::StaffLines* firstSL = system->firstMeasure()->staffLines(staffIndex)->clone();
                Ms::StaffLines* lastSL =  system->lastMeasure()->staffLines(staffIndex);

                qreal lastX =  lastSL->bbox().right()
                              + lastSL->pagePos().x()
                              - firstSL->pagePos().x();
                QVector<QLineF>& lines = firstSL->getLines();
                for (int l = 0, c = lines.size(); l < c; l++) {
                    lines[l].setP2(QPointF(lastX, lines[l].p2().y()));
                }

                printer.setElement(firstSL);
                Ms::paintElement(painter, firstSL);
            }
        }
    }

    // 2nd pass: the rest of the elements
    QList<Ms::Element*> elements = page->elements();
    std::stable_sort(elements.begin(), elements.end(), Ms::elementLessThan);

    int lastNoteIndex = -1;
    for (int i = 0; i < PAGE_NUMBER; ++i) {
        for (const Ms::Element* element: pages[i]->elements()) {
            if (element->type() == Ms::ElementType::NOTE) {
                lastNoteIndex++;
            }
        }
    }

    NotesColors notesColors = parseNotesColors(options.value(OptionKey::NOTES_COLORS, Val()).toQVariant());

    for (const Ms::Element* element : elements) {
        // Always exclude invisible elements
        if (!element->visible()) {
            continue;
        }

        Ms::ElementType type = element->type();
        switch (type) { // In future sub-type code, this switch() grows, and eType gets used
        case Ms::ElementType::STAFF_LINES: // Handled in the 1st pass above
            continue; // Exclude from 2nd pass
            break;
        default:
            break;
        }

        // Set the Element pointer inside SvgGenerator/SvgPaintEngine
        printer.setElement(element);

        // Paint it
        if (element->type() == Ms::ElementType::NOTE && !notesColors.isEmpty()) {
            QColor color = element->color();
            int currentNoteIndex = (++lastNoteIndex);

            if (notesColors.contains(currentNoteIndex)) {
                color = notesColors[currentNoteIndex];
            }

            Ms::Element* note = dynamic_cast<const Ms::Note*>(element)->clone();
            note->setColor(color);
            Ms::paintElement(painter, note);
            delete note;
        } else {
            Ms::paintElement(painter, element);
        }
    }

    painter.endDraw(); // Writes MuseScore SVG file to disk, finally

    // Clean up and return
    Ms::MScore::pixelRatio = pixelRationBackup;
    score->setPrinting(false);
    Ms::MScore::pdfPrinting = false;
    Ms::MScore::svgPrinting = false;

    return true;
}

SvgWriter::NotesColors SvgWriter::parseNotesColors(const QVariant& obj) const
{
    QVariantMap map = obj.toMap();
    NotesColors result;

    for (const QString& noteNumber : map.keys()) {
        result[noteNumber.toInt()] = map[noteNumber].value<QColor>();
    }

    return result;
}
