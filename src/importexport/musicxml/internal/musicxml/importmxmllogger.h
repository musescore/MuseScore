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

#include <QString>

class QXmlStreamReader;

namespace mu::engraving {
class MxmlLogger
{
public:
    enum class Level : char {
        MXML_TRACE, MXML_INFO, MXML_ERROR
    };
    MxmlLogger() {}
    void logDebugTrace(const QString& trace, const QXmlStreamReader* const xmlreader = 0);
    void logDebugInfo(const QString& info, const QXmlStreamReader* const xmlreader = 0);
    void logError(const QString& error, const QXmlStreamReader* const xmlreader = 0);
    void setLoggingLevel(const Level level) { _level = level; }
private:
    Level _level = Level::MXML_INFO;
};
} // namespace Ms

#endif
