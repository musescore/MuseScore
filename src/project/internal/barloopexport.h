/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <memory>
#include <vector>

#include "iexportprojectscenario.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::project {
struct BarLoopPrepared {
    // Scores outlive the notations that point at them.
    std::vector<std::unique_ptr<mu::engraving::MasterScore>> scores;
    notation::INotationPtrList notations;
    mu::engraving::MasterScore* playbackScore = nullptr;
};

std::unique_ptr<BarLoopPrepared> prepareBarLoopExport(notation::IMasterNotationPtr master,
                                                      const notation::INotationPtrList& notations,
                                                      const SelectedBarsExport& selected);
}
