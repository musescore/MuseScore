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

#include "positionswriter.h"

#include <cmath>
#include <QBuffer>

#include "global/io/buffer.h"
#include "global/serialization/xmlstreamwriter.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/system.h"

#include "engraving/types/types.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::io;

static constexpr std::string_view SCORE_TAG("score");
static constexpr std::string_view ELEMENTS_TAG("elements");
static constexpr std::string_view ELEMENT_TAG("element");
static constexpr std::string_view EVENTS_TAG("events");
static constexpr std::string_view EVENT_TAG("event");

static void writeElementPosition(XmlStreamWriter& writer, const std::string& id, const muse::PointF& pos, const muse::PointF& sPos,
                                 page_idx_t pageIndex)
{
    XmlStreamWriter::Attributes attributes;
    attributes.emplace_back("id", id);
    attributes.emplace_back("x", pos.x());
    attributes.emplace_back("y", pos.y());
    attributes.emplace_back("sx", sPos.x());
    attributes.emplace_back("sy", sPos.y());
    attributes.emplace_back("page", pageIndex);

    writer.startElement(ELEMENT_TAG, attributes);
    writer.endElement();
}

static void writeEventPosition(XmlStreamWriter& writer, const std::string& id, int time)
{
    XmlStreamWriter::Attributes attributes;
    attributes.emplace_back("elid", id);
    attributes.emplace_back("position", time);

    writer.startElement(EVENT_TAG, attributes);
    writer.endElement();
}

static void writeMeasureEvents(XmlStreamWriter& writer, Measure* m, int offset, const QHash<void*, int>& segments)
{
    for (mu::engraving::Segment* s = m->first(mu::engraving::SegmentType::ChordRest); s;
         s = s->next(mu::engraving::SegmentType::ChordRest)) {
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

Ret PositionsWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    auto buf = Buffer::opened(IODevice::WriteOnly);
    XmlStreamWriter writer(&buf);

    writer.startDocument();
    writer.startElement(SCORE_TAG);

    writeElementsPositions(writer, score);
    writeEventsPositions(writer, score);

    writer.endElement();
    writer.flush();

    destinationDevice.write(buf.data());

    return true;
}

Ret PositionsWriter::writeList(const INotationPtrList&, io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

qreal PositionsWriter::pngDpiResolution() const
{
    return (imagesExportConfiguration()->exportPngDpiResolution() / mu::engraving::DPI) * 12.0;
}

QHash<void*, int> PositionsWriter::elementIds(const mu::engraving::Score* score) const
{
    QHash<void*, int> elementIds;

    int id = 0;
    if (m_elementType == ElementType::SEGMENT) {
        Measure* m = score->firstMeasureMM();
        for (mu::engraving::Segment* s = (m ? m->first(mu::engraving::SegmentType::ChordRest) : nullptr);
             s; s = s->next1MM(mu::engraving::SegmentType::ChordRest)) {
            elementIds[(void*)s] = id++;
        }
    } else {
        for (Measure* m = score->firstMeasureMM(); m; m = m->nextMeasureMM()) {
            elementIds[(void*)m] = id++;
        }
    }

    return elementIds;
}

void PositionsWriter::writeElementsPositions(XmlStreamWriter& writer, const mu::engraving::Score* score) const
{
    writer.startElement(ELEMENTS_TAG);

    switch (m_elementType) {
    case ElementType::SEGMENT:
        writeSegmentsPositions(writer, score);
        break;
    case ElementType::MEASURE:
        writeMeasuresPositions(writer, score);
        break;
    }

    writer.endElement();
}

void PositionsWriter::writeSegmentsPositions(XmlStreamWriter& writer, const mu::engraving::Score* score) const
{
    int id = 0;
    qreal ndpi = pngDpiResolution();

    Measure* measure = score->firstMeasureMM();
    for (mu::engraving::Segment* segment = (measure ? measure->first(mu::engraving::SegmentType::ChordRest) : nullptr);
         segment; segment = segment->next1MM(mu::engraving::SegmentType::ChordRest)) {
        qreal sx = 0;
        size_t tracks = score->nstaves() * mu::engraving::VOICES;
        for (size_t track = 0; track < tracks; track++) {
            EngravingItem* e = segment->element(static_cast<int>(track));
            if (e) {
                sx = qMax(sx, e->width());
            }
        }

        sx *= ndpi;
        qreal sy = segment->measure()->system()->height() * ndpi;

        int x = segment->pagePos().x() * ndpi;
        int y = segment->pagePos().y() * ndpi;

        Page* page = segment->measure()->system()->page();
        page_idx_t pageIndex = score->pageIdx(page);

        writeElementPosition(writer, std::to_string(id), PointF(x, y), PointF(sx, sy), pageIndex);

        id++;
    }
}

void PositionsWriter::writeMeasuresPositions(XmlStreamWriter& writer, const mu::engraving::Score* score) const
{
    int id = 0;
    qreal ndpi = pngDpiResolution();

    for (Measure* measure = score->firstMeasureMM(); measure; measure = measure->nextMeasureMM()) {
        qreal sx = measure->ldata()->bbox().width() * ndpi;
        qreal sy = measure->system()->height() * ndpi;
        qreal x = measure->pagePos().x() * ndpi;
        qreal y = measure->system()->pagePos().y() * ndpi;

        Page* page = measure->system()->page();
        page_idx_t pageIndex = score->pageIdx(page);

        writeElementPosition(writer, std::to_string(id), PointF(x, y), PointF(sx, sy), pageIndex);

        id++;
    }
}

void PositionsWriter::writeEventsPositions(XmlStreamWriter& writer, const mu::engraving::Score* score) const
{
    QHash<void*, int> elementIds = this->elementIds(score);

    writer.startElement(EVENTS_TAG);

    score->masterScore()->setExpandRepeats(true);

    for (const mu::engraving::RepeatSegment* repeatSegment : score->repeatList()) {
        int startTick = repeatSegment->tick;
        int endTick = repeatSegment->endTick();
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

    writer.endElement();
}
