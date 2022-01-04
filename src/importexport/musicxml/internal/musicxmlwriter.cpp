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

#include "musicxmlwriter.h"

#include "log.h"

#include "libmscore/masterscore.h"
#include "musicxml/exportxml.h"

using namespace mu::iex::musicxml;
using namespace mu::project;
using namespace mu::io;

std::vector<INotationWriter::UnitType> MusicXmlWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool MusicXmlWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret MusicXmlWriter::write(notation::INotationPtr notation, Device& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    return Ms::saveXml(score, &destinationDevice);
}

mu::Ret MusicXmlWriter::writeList(const notation::INotationPtrList&, io::Device&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void MusicXmlWriter::abort()
{
    NOT_IMPLEMENTED;
}

mu::framework::ProgressChannel MusicXmlWriter::progress() const
{
    NOT_IMPLEMENTED;
    static framework::ProgressChannel prog;
    return prog;
}
