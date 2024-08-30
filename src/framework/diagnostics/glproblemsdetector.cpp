/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "glproblemsdetector.h"

#include <QFile>

#include "global/internal/globalconfiguration.h"
#include "global/logger.h"

#include "log.h"

using namespace muse::diagnostics;
using namespace muse::logger;

static const std::string BAD_MESSAGE = "Failed to build graphics pipeline state";

//! NOTE This is message just for test
//static const std::string BAD_MESSAGE = "QML QQuickItem: Binding loop detected for property ";

namespace muse::diagnostics {
class GlLogDest : public LogDest
{
public:

    GlLogDest()
        : LogDest(LogLayout("")) {}

    std::string name() const { return "GlProblemsDetector"; }
    void write(const LogMsg& logMsg)
    {
        if (isProblemDetected) {
            return;
        }

        if (logMsg.message.find(BAD_MESSAGE) != std::string::npos) {
            isProblemDetected = true;
            std::cout << "found message: " << BAD_MESSAGE << std::endl;
        }
    }

    bool isProblemDetected = false;
};
}

GlProblemsDetector::GlProblemsDetector(const Version& appVersion)
    : m_appVersion(appVersion)
{
    Logger* logger = Logger::instance();

    m_logDest = new GlLogDest();
    logger->addDest(m_logDest);
}

GlProblemsDetector::~GlProblemsDetector()
{
    Logger* logger = Logger::instance();
    logger->removeDest(m_logDest);
    delete m_logDest;
}

QString GlProblemsDetector::softwareMarkerFilePath() const
{
    GlobalConfiguration conf(modularity::globalCtx());
    return conf.userAppDataPath().toQString()
           + "/need_use_software_quick_backend_"
           + m_appVersion.toString()
           + ".dat";
}

void GlProblemsDetector::setIsNeedUseSoftwareRender(bool arg)
{
    if (arg) {
        QFile file(softwareMarkerFilePath());
        file.open(QIODevice::WriteOnly); // create file
    } else {
        QFile::remove(softwareMarkerFilePath());
    }
}

bool GlProblemsDetector::isNeedUseSoftwareRender() const
{
    return QFile::exists(softwareMarkerFilePath());
}

bool GlProblemsDetector::isProblemWithGl() const
{
    return m_logDest->isProblemDetected;
}
