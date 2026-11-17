/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "modularity/ioc.h"
#include "types/string.h"
#include "types/ret.h"
#include "engraving/engravingerrors.h"

namespace mu::engraving {
class MasterScore;
class Score;
}

namespace mu::iex::musicxml {
muse::Ret importMusicXmlfromBuffer(engraving::Score* score, const muse::String& name, const muse::ByteArray& data, bool forceMode);
muse::Ret importCompressedMusicXml(engraving::MasterScore* score, const muse::String& name, bool forceMode);
muse::Ret importMusicXml(engraving::MasterScore* score, const muse::String& name, bool forceMode);
}
