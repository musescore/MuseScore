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
#ifndef MU_CONVERTER_ICONVERTERCONTROLLER_H
#define MU_CONVERTER_ICONVERTERCONTROLLER_H

#include "modularity/imoduleinterface.h"
#include "types/ret.h"
#include "io/path.h"

namespace mu::converter {
class IConverterController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IConverterController)
public:
    virtual ~IConverterController() = default;

    virtual Ret fileConvert(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath = io::path_t(),
                            bool forceMode = false) = 0;
    virtual Ret batchConvert(const io::path_t& batchJobFile, const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;
    virtual Ret convertScoreParts(const io::path_t& in, const io::path_t& out,
                                  const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;

    virtual Ret exportScoreMedia(const io::path_t& in, const io::path_t& out,
                                 const io::path_t& highlightConfigPath = io::path_t(),
                                 const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;
    virtual Ret exportScoreMeta(const io::path_t& in, const io::path_t& out,
                                const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;
    virtual Ret exportScoreParts(const io::path_t& in, const io::path_t& out,
                                 const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;
    virtual Ret exportScorePartsPdfs(const io::path_t& in, const io::path_t& out,
                                     const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;
    virtual Ret exportScoreTranspose(const io::path_t& in, const io::path_t& out, const std::string& optionsJson,
                                     const io::path_t& stylePath = io::path_t(), bool forceMode = false) = 0;

    virtual Ret exportScoreVideo(const io::path_t& in, const io::path_t& out) = 0;

    virtual Ret updateSource(const io::path_t& in, const std::string& newSource, bool forceMode = false) = 0;
};
}

#endif // MU_CONVERTER_ICONVERTERCONTROLLER_H
