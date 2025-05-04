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

#include "musx/musx.h"

namespace mu::iex::finale {
class EnigmaXmlImporter;

class FinaleLogger
{
public:
    enum class Level : char {
        MUSX_TRACE, MUSX_INFO, MUSX_WARNING, MUSX_ERROR
    };
    FinaleLogger();
    ~FinaleLogger();

    // FinaleLogger must not be copied because it takes ownership of the musx logging function.
    FinaleLogger(const FinaleLogger&) = delete;
    FinaleLogger& operator=(const FinaleLogger&) = delete;
    FinaleLogger(FinaleLogger&&) = delete;
    FinaleLogger& operator=(FinaleLogger&&) = delete;

    void logDebugTrace(const muse::String& trace, const musx::dom::DocumentPtr& musxDocument = nullptr, musx::dom::MeasCmper currMusxStaff = 0, musx::dom::InstCmper currMusxMeas = 0) const;
    void logInfo(const muse::String& info, const musx::dom::DocumentPtr& musxDocument = nullptr, musx::dom::MeasCmper currMusxStaff = 0, musx::dom::InstCmper currMusxMeas = 0) const;
    void logWarning(const muse::String& warning, const musx::dom::DocumentPtr& musxDocument = nullptr, musx::dom::MeasCmper currMusxStaff = 0, musx::dom::InstCmper currMusxMeas = 0) const;
    void logError(const muse::String& error, const musx::dom::DocumentPtr& musxDocument = nullptr, musx::dom::MeasCmper currMusxStaff = 0, musx::dom::InstCmper currMusxMeas = 0) const;
    void setLoggingLevel(const Level level) { m_level = level; }
private:
    Level m_level = Level::MUSX_INFO;
};
} // namespace Ms
