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

#include "../abstractnotationwriter.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::framework;

std::vector<WriterUnitType> AbstractNotationWriter::supportedUnitTypes() const
{
    return { WriterUnitType::PER_PART };
}

bool AbstractNotationWriter::supportsUnitType(WriterUnitType unitType) const
{
    std::vector<WriterUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret AbstractNotationWriter::write(const INotationPtrList&, system::IODevice&, const Options& options)
{
    if (supportsUnitType(static_cast<WriterUnitType>(options.value(OptionKey::UNIT_TYPE, Val(0)).toInt()))) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void AbstractNotationWriter::abort()
{
    NOT_IMPLEMENTED;
}

ProgressChannel AbstractNotationWriter::progress() const
{
    return m_progress;
}

WriterUnitType AbstractNotationWriter::unitTypeFromOptions(const Options& options) const
{
    std::vector<WriterUnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return WriterUnitType::PER_PART;
    }

    WriterUnitType defaultUnitType = supported.front();
    WriterUnitType unitType = static_cast<WriterUnitType>(options.value(OptionKey::UNIT_TYPE, Val(
                                                                            static_cast<int>(defaultUnitType))).toInt());
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
