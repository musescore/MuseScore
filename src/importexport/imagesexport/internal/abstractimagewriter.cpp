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
#include "abstractimagewriter.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;

std::vector<INotationWriter::UnitType> AbstractImageWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool AbstractImageWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret AbstractImageWriter::write(INotationPtr, io::Device&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) != UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(static_cast<UnitType>(options.value(OptionKey::UNIT_TYPE, Val(0)).toInt()))) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

mu::Ret AbstractImageWriter::writeList(const INotationPtrList&, io::Device&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(static_cast<UnitType>(options.value(OptionKey::UNIT_TYPE, Val(0)).toInt()))) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void AbstractImageWriter::abort()
{
    NOT_IMPLEMENTED;
}

mu::framework::ProgressChannel AbstractImageWriter::progress() const
{
    return m_progress;
}

INotationWriter::UnitType AbstractImageWriter::unitTypeFromOptions(const Options& options) const
{
    std::vector<UnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return UnitType::PER_PART;
    }

    UnitType defaultUnitType = supported.front();
    UnitType unitType = static_cast<UnitType>(options.value(OptionKey::UNIT_TYPE, Val(static_cast<int>(defaultUnitType))).toInt());
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
