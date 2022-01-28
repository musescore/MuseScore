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

#include "mscnotationwriter.h"

#include "engraving/engravingproject.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::framework;
using namespace mu::notation;
using namespace mu::project;

MscNotationWriter::MscNotationWriter(engraving::MscIoMode mode)
    : m_mode(mode)
{
}

std::vector<INotationWriter::UnitType> MscNotationWriter::supportedUnitTypes() const
{
    return { UnitType::MULTI_PART };
}

bool MscNotationWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret MscNotationWriter::write(INotationPtr notation, io::Device& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    MscWriter::Params params;
    params.mode = m_mode;

    params.filePath = destinationDevice.property("path").toString();
    if (m_mode != MscIoMode::Dir) {
        params.device = &destinationDevice;
    } else if (QFile::exists(params.filePath)) {
        QFile::remove(params.filePath);
    }

    MscWriter msczWriter(params);
    if (!msczWriter.open()) {
        LOGE() << "MscWriter is not opened";
        return Ret(Ret::Code::UnknownError);
    }
    notation->elements()->msScore()->masterScore()->project().lock()->writeMscz(msczWriter, false, true);
    return Ret(Ret::Code::Ok);
}

mu::Ret MscNotationWriter::writeList(const INotationPtrList&, io::Device&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void MscNotationWriter::abort()
{
    NOT_IMPLEMENTED;
}

ProgressChannel MscNotationWriter::progress() const
{
    NOT_IMPLEMENTED;
    static ProgressChannel prog;
    return prog;
}
