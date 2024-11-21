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

#pragma once

#include "serialization/xmlstreamreader.h"

namespace mu::iex::musicxml {
class MusicXmlLogger
{
public:
    enum class Level : char {
        MXML_TRACE, MXML_INFO, MXML_ERROR
    };
    MusicXmlLogger() {}
    void logDebugTrace(const muse::String& trace, const muse::XmlStreamReader* xmlreader = 0);
    void logDebugInfo(const muse::String& info, const muse::XmlStreamReader* xmlreader = 0);
    void logError(const muse::String& error, const muse::XmlStreamReader* xmlreader = 0);
    void setLoggingLevel(const Level level) { m_level = level; }
private:
    Level m_level = Level::MXML_INFO;
};
} // namespace Ms
