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
#include <optional>

#include "internal/importfinalelogger.h"
#include "internal/importfinalescoremap.h"

#include "log.h"

using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

FinaleLogger::FinaleLogger()
{
    musx::util::Logger::setCallback([this](musx::util::Logger::LogLevel logLevel, const std::string& msg) {
        String msgStr = String::fromUtf8(msg.c_str());
        switch (logLevel) {
        default:
        case musx::util::Logger::LogLevel::Info:
            this->logInfo(msgStr);
            break;
        case musx::util::Logger::LogLevel::Warning:
            this->logWarning(msgStr);
            break;
        case musx::util::Logger::LogLevel::Error:
            this->logError(msgStr);
            break;
        case musx::util::Logger::LogLevel::Verbose:
            this->logDebugTrace(msgStr);
        }
    });
}

FinaleLogger::~FinaleLogger()
{
    musx::util::Logger::setCallback(musx::util::Logger::LogCallback());
}

//---------------------------------------------------------
//   musxLocation
//---------------------------------------------------------

static String musxLocation(const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas)
{
    String loc;
    if (musxDocument && currMusxStaff > 0 && currMusxMeas > 0) {
        std::string staffName = [&]() -> std::string {
            if (auto staff = others::StaffComposite::createCurrent(musxDocument, SCORE_PARTID, currMusxStaff, currMusxMeas, 0)) {
                auto instName = staff->getFullInstrumentName();
                if (!instName.empty()) return instName;
            }
            return "Staff " + std::to_string(currMusxStaff);
        }();
        loc = String::fromUtf8(("[" + staffName + " m" + std::to_string(currMusxMeas) + "] ").c_str());
    }
    return loc;
}

//---------------------------------------------------------
//   logString
//---------------------------------------------------------
static String logString(FinaleLogger::Level level, const String& text, const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas)
{
    String str;
    switch (level) {
    case FinaleLogger::Level::MUSX_TRACE: str = u"Trace";
        break;
    case FinaleLogger::Level::MUSX_INFO: str = u"Info";
        break;
    case FinaleLogger::Level::MUSX_WARNING: str = u"Warning";
        break;
    case FinaleLogger::Level::MUSX_ERROR: str = u"Error";
        break;
    default: str = u"Unknown";
        break;
    }

    str += musxLocation(musxDocument, currMusxStaff, currMusxMeas);
    str += u": ";
    str += text;

    return str;
}

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------

/**
 Log debug (function) trace.
 */

void FinaleLogger::logDebugTrace(const String& trace, const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas) const
{
    if (m_level <= Level::MUSX_TRACE) {
        LOGD() << logString(Level::MUSX_TRACE, trace, musxDocument, currMusxStaff, currMusxMeas);
    }
}

//---------------------------------------------------------
//   logInfo
//---------------------------------------------------------

/**
 Log debug \a info (non-fatal events).
 */

void FinaleLogger::logInfo(const String& info, const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas) const
{
    if (m_level <= Level::MUSX_INFO) {
        LOGI() << logString(Level::MUSX_INFO, info, musxDocument, currMusxStaff, currMusxMeas);
    }
}

//---------------------------------------------------------
//   logWarning
//---------------------------------------------------------

/**
 Log \a warning (possible error but non-fatal).
 */

void FinaleLogger::logWarning(const String& warning, const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas) const
{
    if (m_level <= Level::MUSX_WARNING) {
        LOGW() << logString(Level::MUSX_WARNING, warning, musxDocument, currMusxStaff, currMusxMeas);
    }
}

//---------------------------------------------------------
//   logError
//---------------------------------------------------------

/**
 Log \a error (fatal errors: file processing stopped).
 */

void FinaleLogger::logError(const String& error, const DocumentPtr& musxDocument, MeasCmper currMusxStaff, InstCmper currMusxMeas) const
{
    if (m_level <= Level::MUSX_ERROR) {
        LOGE() << logString(Level::MUSX_ERROR, error, musxDocument, currMusxStaff, currMusxMeas);
    }
}

}
