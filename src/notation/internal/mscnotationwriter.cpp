/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "io/file.h"

#include "engraving/engravingproject.h"
#include "engraving/dom/masterscore.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

MscNotationWriter::MscNotationWriter(engraving::MscIoMode mode)
    : m_mode(mode)
{
}

std::vector<WriteUnitType> MscNotationWriter::supportedUnitTypes() const
{
    return { WriteUnitType::MULTI_PART };
}

bool MscNotationWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret MscNotationWriter::write(INotationProjectPtr project, io::IODevice& device, const WriteOptions& /*options*/)
{
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = project->masterNotation()->notation()->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    MscWriter::Params params;
    params.mode = m_mode;
    params.filePath = device.meta("file_path");
    if (m_mode == MscIoMode::Dir) {
        IF_ASSERT_FAILED(!params.filePath.empty()) {
            return make_ret(Ret::Code::InternalError);
        }

        if (File::exists(params.filePath)) {
            File::remove(params.filePath);
        }
    } else {
        params.device = &device;
    }

    MscWriter msczWriter(params);
    if (!msczWriter.open()) {
        LOGE() << "MscWriter is not opened";
        return Ret(Ret::Code::UnknownError);
    }

    score->masterScore()->project().lock()->writeMscz(msczWriter, true);

    msczWriter.close();

    if (msczWriter.hasError()) {
        LOGE() << "MscWriter has error";
        return Ret(Ret::Code::UnknownError);
    }

    return Ret(Ret::Code::Ok);
}

Ret MscNotationWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    file.setMeta("file_path", filePath.toStdString());
    if (!file.open(IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();
    return ret;
}
