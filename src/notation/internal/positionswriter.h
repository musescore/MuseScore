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

#ifndef MU_NOTATION_POSITIONSWRITER_H
#define MU_NOTATION_POSITIONSWRITER_H

#include "modularity/ioc.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "project/inotationwriter.h"

namespace mu::deprecated {
class XmlWriter;
}

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class PositionsWriter : public project::INotationWriter
{
    INJECT(iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)

public:
    enum class ElementType {
        SEGMENT,
        MEASURE
    };

    explicit PositionsWriter() = default;
    explicit PositionsWriter(ElementType elementType);

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    Ret write(notation::INotationPtr notation, io::IODevice& device, const Options& options = Options()) override;
    Ret writeList(const INotationPtrList& notations, io::IODevice& device, const Options& options = Options()) override;

private:
    qreal pngDpiResolution() const;
    QHash<void*, int> elementIds(const mu::engraving::Score* score) const;

    void writeElementsPositions(deprecated::XmlWriter& writer, const mu::engraving::Score* score) const;
    void writeSegmentsPositions(deprecated::XmlWriter& writer, const mu::engraving::Score* score) const;
    void writeMeasuresPositions(deprecated::XmlWriter& writer, const mu::engraving::Score* score) const;

    void writeEventsPositions(deprecated::XmlWriter& writer, const mu::engraving::Score* score) const;

    ElementType m_elementType = ElementType::SEGMENT;
};
}

#endif // MU_NOTATION_POSITIONSWRITER_H
