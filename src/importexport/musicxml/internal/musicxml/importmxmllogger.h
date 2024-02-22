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

#ifndef __IMPORTMXMLLOGGER_H__
#define __IMPORTMXMLLOGGER_H__

#include "global/types/string.h"

namespace mu {
class XmlStreamReader;
}

namespace mu::engraving {
class MxmlLogger
{
public:
    enum class Level : char {
        MXML_TRACE, MXML_INFO, MXML_ERROR
    };
    MxmlLogger() {}
    void logDebugTrace(const mu::String& trace, const XmlStreamReader* xmlreader = 0);
    void logDebugInfo(const String& info, const XmlStreamReader* xmlreader = 0);
    void logError(const String& error, const XmlStreamReader* xmlreader = 0);
    void setLoggingLevel(const Level level) { m_level = level; }
private:
    Level m_level = Level::MXML_INFO;
};
} // namespace Ms

#endif
