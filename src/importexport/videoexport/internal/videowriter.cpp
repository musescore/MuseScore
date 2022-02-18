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
#include "videowriter.h"

#include "log.h"

using namespace mu::iex::videoexport;
using namespace mu::project;

std::vector<INotationWriter::UnitType> VideoWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool VideoWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret VideoWriter::write(notation::INotationPtr notation, io::Device& device, const Options& options)
{
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::NotImplemented);
}

mu::Ret VideoWriter::writeList(const notation::INotationPtrList& notations, io::Device& device, const Options& options)
{
    UNUSED(notations);
    UNUSED(device);
    UNUSED(options);
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::NotImplemented);
}

void VideoWriter::abort()
{
    NOT_IMPLEMENTED;
}

mu::framework::ProgressChannel VideoWriter::progress() const
{
    static mu::framework::ProgressChannel progressCh;
    return progressCh;
}
