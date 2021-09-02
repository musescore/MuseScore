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

#include "positionswriter.h"

#include <cmath>

#include "libmscore/system.h"
#include "libmscore/repeatlist.h"

#include "log.h"
#include "global/xmlwriter.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;
using namespace mu::framework;

constexpr std::string_view SCORE_TAG("score");
constexpr std::string_view ELEMENT_TAG("elements");
constexpr std::string_view ELEMENTS_TAG("elements");
constexpr std::string_view EVENTS_TAG("events");

static void writeElementPosition(XmlWriter& writer, const std::string& id, const mu::PointF& pos, const mu::PointF& sPos,
                                 int pageIndex)
{
    writer.writeStartElement(ELEMENT_TAG);
    writer.writeAttribute("id", id);
    writer.writeAttribute("x", std::to_string(pos.x()));
    writer.writeAttribute("y", std::to_string(pos.y()));
    writer.writeAttribute("sx", std::to_string(sPos.x()));
    writer.writeAttribute("sy", std::to_string(sPos.y()));
    writer.writeAttribute("page", std::to_string(pageIndex));
    writer.writeEndElement();
}

static void writeEventPosition(XmlWriter& writer, const std::string& id, int time)
{
    writer.writeStartElement(EVENTS_TAG);
    writer.writeAttribute("elid", id);
    writer.writeAttribute("position", std::to_string(time));
    writer.writeEndElement();
}

static void writeMeasureEvents(XmlWriter& writer, Measure* m, int offset, const QHash<void*, int>& segments)
{
    for (Ms::Segment* s = m->first(Ms::SegmentType::ChordRest); s; s = s->next(Ms::SegmentType::ChordRest)) {
        int tick = s->tick().ticks() + offset;
        int id = segments[(void*)s];
        int time = lrint(m->score()->repeatList().utick2utime(tick) * 1000);

        writeEventPosition(writer, std::to_string(id), time);
    }
}

PositionsWriter::PositionsWriter(PositionsWriter::ElementType elementType)
    : m_elementType(elementType)
{
}

std::vector<INotationWriter::UnitType> PositionsWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool PositionsWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret PositionsWriter::write(INotationPtr notation, Device& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QHash<void*, int> segments;

    XmlWriter writer(&destinationDevice);

    writer.writeStartDocument();
    writer.writeStartElement(SCORE_TAG);

    writeElementsPositions(writer, score);
    writeEventsPositions(writer, score);

    writer.writeEndElement();
    writer.writeEndDocument();

    return true;
}

mu::Ret PositionsWriter::writeList(const INotationPtrList&, io::Device&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void PositionsWriter::abort()
{
    NOT_IMPLEMENTED;
}

ProgressChannel PositionsWriter::progress() const
{
    NOT_IMPLEMENTED;
    static ProgressChannel prog;
    return prog;
}

qreal PositionsWriter::pngDpiResolution() const
{
    return (imagesExportConfiguration()->exportPngDpiResolution() / Ms::DPI) * 12.0;
}

QHash<void*, int> PositionsWriter::elementIds(const Ms::Score* score) const
{
    QHash<void*, int> elementIds;

    int id = 0;
    if (m_elementType == ElementType::SEGMENT) {
        Measure* m = score->firstMeasureMM();
        for (Ms::Segment* s = (m ? m->first(Ms::SegmentType::ChordRest) : nullptr);
             s; s = s->next1MM(Ms::SegmentType::ChordRest)) {
            elementIds[(void*)s] = id++;
        }
    } else {
        for (Measure* m = score->firstMeasureMM(); m; m = m->nextMeasureMM()) {
            elementIds[(void*)m] = id++;
        }
    }

    return elementIds;
}

void PositionsWriter::writeElementsPositions(XmlWriter& writer, const Ms::Score* score) const
{
    writer.writeStartElement(ELEMENTS_TAG);

    switch (m_elementType) {
    case ElementType::SEGMENT:
        writeSegmentsPositions(writer, score);
        break;
    case ElementType::MEASURE:
        writeMeasuresPositions(writer, score);
        break;
    }

    writer.writeEndElement();
}

void PositionsWriter::writeSegmentsPositions(XmlWriter& writer, const Ms::Score* score) const
{
    int id = 0;
    qreal ndpi = pngDpiResolution();

    Measure* measure = score->firstMeasureMM();
    for (Ms::Segment* segment = (measure ? measure->first(Ms::SegmentType::ChordRest) : nullptr);
         segment; segment = segment->next1MM(Ms::SegmentType::ChordRest)) {
        qreal sx = 0;
        int tracks = score->nstaves() * Ms::VOICES;
        for (int track = 0; track < tracks; track++) {
            EngravingItem* e = segment->element(track);
            if (e) {
                sx = qMax(sx, e->width());
            }
        }

        sx *= ndpi;
        qreal sy = segment->measure()->system()->height() * ndpi;

        int x = segment->pagePos().x() * ndpi;
        int y = segment->pagePos().y() * ndpi;

        Page* page = segment->measure()->system()->page();
        int pageIndex = score->pageIdx(page);

        writeElementPosition(writer, std::to_string(id), PointF(x, y), PointF(sx, sy), pageIndex);

        id++;
    }
}

void PositionsWriter::writeMeasuresPositions(XmlWriter& writer, const Ms::Score* score) const
{
    int id = 0;
    qreal ndpi = pngDpiResolution();

    for (Measure* measure = score->firstMeasureMM(); measure; measure = measure->nextMeasureMM()) {
        qreal sx = measure->bbox().width() * ndpi;
        qreal sy = measure->system()->height() * ndpi;
        qreal x = measure->pagePos().x() * ndpi;
        qreal y = measure->system()->pagePos().y() * ndpi;

        Page* page = measure->system()->page();
        int pageIndex = score->pageIdx(page);

        writeElementPosition(writer, std::to_string(id), PointF(x, y), PointF(sx, sy), pageIndex);

        id++;
    }
}

void PositionsWriter::writeEventsPositions(XmlWriter& writer, const Ms::Score* score) const
{
    QHash<void*, int> elementIds = this->elementIds(score);

    writer.writeStartElement(EVENTS_TAG);

    score->masterScore()->setExpandRepeats(true);

    for (const Ms::RepeatSegment* repeatSegment : score->repeatList()) {
        int startTick = repeatSegment->tick;
        int endTick = startTick + repeatSegment->len();
        int tickOffset = repeatSegment->utick - repeatSegment->tick;
        for (Measure* measure = score->tick2measureMM(Fraction::fromTicks(startTick)); measure; measure = measure->nextMeasureMM()) {
            if (m_elementType == ElementType::SEGMENT) {
                writeMeasureEvents(writer, measure, tickOffset, elementIds);
            } else {
                int tick = measure->tick().ticks() + tickOffset;
                int id = elementIds[(void*)measure];
                int time = std::lrint(measure->score()->repeatList().utick2utime(tick) * 1000);

                writeEventPosition(writer, std::to_string(id), time);
            }

            if (measure->endTick().ticks() >= endTick) {
                break;
            }
        }
    }

    writer.writeEndElement();
}
