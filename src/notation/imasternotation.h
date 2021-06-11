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
#ifndef MU_NOTATION_IMASTERNOTATION_H
#define MU_NOTATION_IMASTERNOTATION_H

#include "modularity/imoduleexport.h"
#include "inotation.h"
#include "iexcerptnotation.h"
#include "retval.h"
#include "io/path.h"
#include "system/iodevice.h"

namespace mu::notation {
using ExcerptNotationList = std::vector<IExcerptNotationPtr>;

class IMasterNotation
{
public:
    virtual INotationPtr notation() = 0;

    virtual Meta metaInfo() const = 0;

    virtual Ret load(const io::path& path) = 0;
    virtual io::path path() const = 0;

    virtual Ret createNew(const ScoreCreateOptions& scoreInfo) = 0;
    virtual RetVal<bool> created() const = 0;

    virtual Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) = 0;
    virtual ValNt<bool> needSave() const = 0;

    virtual ValCh<ExcerptNotationList> excerpts() const = 0;
    virtual void setExcerpts(const ExcerptNotationList& excerpts) = 0;

    virtual INotationPartsPtr parts() const = 0;
    virtual INotationPtr clone() const = 0;

    virtual Ret writeToDevice(system::IODevice& destinationDevice) = 0;
};

using IMasterNotationPtr = std::shared_ptr<IMasterNotation>;
}

#endif // MU_NOTATION_IMASTERNOTATION_H
