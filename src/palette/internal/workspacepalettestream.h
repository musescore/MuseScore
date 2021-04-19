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
#ifndef MU_PALETTE_WORKSPACEPALETTESTREAM_H
#define MU_PALETTE_WORKSPACEPALETTESTREAM_H

#include "workspace/iworkspacedatastream.h"

#include "palettetypes.h"

namespace Ms {
class XmlReader;
}

namespace mu::palette {
class WorkspacePaletteStream : public workspace::IWorkspaceDataStream
{
public:
    workspace::AbstractDataPtrList read(system::IODevice& sourceDevice) const override;
    void write(const workspace::AbstractDataPtrList& dataList, system::IODevice& destinationDevice) const override;

    workspace::WorkspaceTag tag() const override;

private:
    PaletteWorkspaceDataPtr readPalettes(Ms::XmlReader& reader) const;
};
}

#endif // MU_PALETTE_WORKSPACEPALETTESTREAM_H
