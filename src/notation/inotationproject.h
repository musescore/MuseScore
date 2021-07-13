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
#ifndef MU_NOTATION_INOTATIONPROJECT_H
#define MU_NOTATION_INOTATIONPROJECT_H

#include <memory>

#include "io/path.h"
#include "ret.h"

#include "imasternotation.h"
#include "iprojectaudiosettings.h"

namespace mu::notation {
class INotationProject
{
public:
    virtual ~INotationProject() = default;

    virtual io::path path() const = 0;

    virtual Ret load(const io::path& path, const io::path& stylePath = io::path(), bool forceMode = false) = 0;
    virtual Ret createNew(const ScoreCreateOptions& scoreInfo) = 0;

    virtual RetVal<bool> created() const = 0;
    virtual ValNt<bool> needSave() const = 0;

    virtual Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) = 0;
    virtual Ret writeToDevice(io::Device& destinationDevice) = 0;

    virtual Meta metaInfo() const = 0;
    virtual void setMetaInfo(const Meta& meta) = 0;

    virtual IMasterNotationPtr masterNotation() const = 0;
    virtual IProjectAudioSettingsPtr audioSettings() const = 0;
};

using INotationProjectPtr = std::shared_ptr<INotationProject>;
}

#endif // MU_NOTATION_INOTATIONPROJECT_H
