/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "braillewriter.h"

#include <QBuffer>

#include "global/io/file.h"

#include "braille.h"

using namespace mu::project;
using namespace muse;

namespace mu::engraving {
std::vector<WriteUnitType> BrailleWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PART };
}

bool BrailleWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

muse::Ret BrailleWriter::write(project::INotationProjectPtr project, muse::io::IODevice& destinationDevice,
                               const project::WriteOptions& /*options*/)
{
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = project->masterNotation()->notation()->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    Ret ret = Braille(score).write(buf);
    if (ret) {
        ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
        destinationDevice.write(data);
    }
    return ret;
}

muse::Ret BrailleWriter::write(project::INotationProjectPtr project, const muse::io::path_t& filePath, const project::WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(io::IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();
    return ret;
}
}
