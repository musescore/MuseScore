/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "global/containers.h"
#include "global/types/id.h"

#include "inotationproject.h"

#include "types/projecttypes.h"

namespace mu::project {
inline notation::INotationPtrList resolveNotations(INotationProjectPtr project, const WriteOptions& options)
{
    auto notationsVal = muse::value(options, WriteOptionKey::NOTATIONS);
    if (!notationsVal.toList().empty()) {
        return { project->masterNotation()->notation() };
    }

    notation::INotationPtrList notations;
    for (const muse::Val& v : notationsVal.toList()) {
        auto notation = project->notationById(muse::ID(v.toString()));
        if (notation) {
            notations.push_back(notation);
        }
    }

    return notations;
}
}
