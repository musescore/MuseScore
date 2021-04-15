/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "workspacepalettestream.h"

#include "workspace/workspacetypes.h"

#include "log.h"

#include "libmscore/xml.h"

using namespace mu::palette;
using namespace mu::workspace;
using namespace mu::system;

static const QString PALETTE_TAG("PaletteBox");

AbstractDataPtrList WorkspacePaletteStream::read(IODevice& sourceDevice) const
{
    Ms::XmlReader reader(&sourceDevice);

    while (!reader.atEnd()) {
        reader.readNextStartElement();

        if (reader.name() == PALETTE_TAG) {
            return { readPalettes(reader) };
        }
    }

    return {};
}

PaletteWorkspaceDataPtr WorkspacePaletteStream::readPalettes(Ms::XmlReader& reader) const
{
    PaletteWorkspaceDataPtr palettes = std::make_shared<PaletteWorkspaceData>();
    palettes->tag = tag();
    palettes->tree = std::make_shared<Ms::PaletteTree>();
    palettes->tree->read(reader);

    return palettes;
}

void WorkspacePaletteStream::write(const AbstractDataPtrList& dataList, IODevice& destinationDevice) const
{
    Ms::XmlWriter writer(nullptr, &destinationDevice);

    for (AbstractDataPtr data : dataList) {
        PaletteWorkspaceDataPtr palettes = std::dynamic_pointer_cast<PaletteWorkspaceData>(data);
        IF_ASSERT_FAILED(palettes) {
            return;
        }
        palettes->tree->write(writer);
    }
}

WorkspaceTag WorkspacePaletteStream::tag() const
{
    return WorkspaceTag::Palettes;
}
