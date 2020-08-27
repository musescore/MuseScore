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
#include "workspacepalettestream.h"
#include <memory>
#include "ptrutils.h"

using namespace mu::palette;
using namespace mu::workspace;

std::shared_ptr<AbstractData> WorkspacePaletteStream::read(Ms::XmlReader& xml) const
{
    std::shared_ptr<PaletteWorkspaceData> data = std::make_shared<PaletteWorkspaceData>();
    data->tree = std::move(std::unique_ptr<Ms::PaletteTree>(new Ms::PaletteTree));
    data->tree->read(xml);
    return data;
}

void WorkspacePaletteStream::write(Ms::XmlWriter& xml, std::shared_ptr<AbstractData> data) const
{
    PaletteWorkspaceData* pdata = ptr::checked_cast<PaletteWorkspaceData>(data.get());
    IF_ASSERT_FAILED(pdata) {
        return;
    }
    pdata->tree->write(xml);
}
