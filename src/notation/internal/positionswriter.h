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

#pragma once

#include "modularity/ioc.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "project/iprojectwriter.h"

namespace muse {
class XmlStreamWriter;
}

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class PositionsWriter : public project::IProjectWriter, public muse::Contextable
{
    muse::GlobalInject<iex::imagesexport::IImagesExportConfiguration> imagesExportConfiguration;

public:
    enum class ElementType {
        SEGMENT,
        MEASURE
    };

    explicit PositionsWriter(const muse::modularity::ContextPtr& ctx);
    explicit PositionsWriter(ElementType elementType, const muse::modularity::ContextPtr& ctx);

    std::vector<project::WriteUnitType> supportedUnitTypes() const override;
    bool supportsUnitType(project::WriteUnitType unitType) const override;

    muse::Ret write(project::INotationProjectPtr project, muse::io::IODevice& device,
                    const project::WriteOptions& options = project::WriteOptions()) override;
    muse::Ret write(project::INotationProjectPtr project, const muse::io::path_t& filePath,
                    const project::WriteOptions& options = project::WriteOptions()) override;

private:
    qreal pngDpiResolution() const;
    QHash<void*, int> elementIds(const mu::engraving::Score* score) const;

    void writeElementsPositions(muse::XmlStreamWriter& writer, const mu::engraving::Score* score) const;
    void writeSegmentsPositions(muse::XmlStreamWriter& writer, const mu::engraving::Score* score) const;
    void writeMeasuresPositions(muse::XmlStreamWriter& writer, const mu::engraving::Score* score) const;

    void writeEventsPositions(muse::XmlStreamWriter& writer, const mu::engraving::Score* score) const;

    ElementType m_elementType = ElementType::SEGMENT;
};
}
