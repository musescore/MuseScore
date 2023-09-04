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

#include "io/buffer.h"
#include "io/file.h"

#include "engraving/engravingproject.h"
#include "engraving/dom/masterscore.h"

#include "log.h"

using namespace mu::io;
using namespace mu::engraving;
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

mu::Ret MscNotationWriter::write(INotationPtr notation, QIODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    Buffer buf;

    MscWriter::Params params;
    params.mode = m_mode;

    params.filePath = destinationDevice.property("path").toString();
    if (m_mode != MscIoMode::Dir) {
        params.device = &buf;
    } else if (File::exists(params.filePath)) {
        File::remove(params.filePath);
    }

    MscWriter msczWriter(params);
    if (!msczWriter.open()) {
        LOGE() << "MscWriter is not opened";
        return Ret(Ret::Code::UnknownError);
    }

    notation->elements()->msScore()->masterScore()->project().lock()->writeMscz(msczWriter, false, true);

    msczWriter.close();

    if (msczWriter.hasError()) {
        LOGE() << "MscWriter has error";
        return Ret(Ret::Code::UnknownError);
    }

    if (m_mode != MscIoMode::Dir) {
        buf.open(io::IODevice::ReadOnly);
        ByteArray ba = buf.readAll();
        destinationDevice.write(reinterpret_cast<const char*>(ba.constData()), ba.size());
        buf.close();
    }

    return Ret(Ret::Code::Ok);
}

mu::Ret MscNotationWriter::writeList(const INotationPtrList&, QIODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}
