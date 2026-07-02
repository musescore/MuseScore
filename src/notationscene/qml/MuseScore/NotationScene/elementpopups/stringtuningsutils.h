/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <QString>

#include "global/types/string.h"
#include "engraving/types/types.h"

namespace mu::notation {
// Utility helpers shared by the string tunings popup model and its tests.
// They keep note-name formatting and parsing aligned with the selected
// chord-symbol spelling without depending on QML state.
QString stringTuningPitchToString(int pitch, bool useFlats, engraving::NoteSpellingType spelling);
int stringTuningInputToPitch(const muse::String& input, engraving::NoteSpellingType spelling, bool* useFlat = nullptr);
}
