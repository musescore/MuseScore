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
#include "gsproblemsdetector.h"

#include <QFile>

#include "global/internal/globalconfiguration.h"
#include "global/logger.h"

#include "log.h"

using namespace muse::diagnostics;
using namespace muse::logger;

static const std::string BAD_MESSAGE = "Failed to build graphics pipeline state";

namespace muse::diagnostics {
class GSLogDest : public LogDest
{
public:

    GSLogDest()
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

GSTestObj* GSProblemDetector::gsTestObj = nullptr;

GSProblemDetector::GSProblemDetector(const Version& appVersion)
    : m_appVersion(appVersion)
{
    Logger* logger = Logger::instance();

    m_logDest = new GSLogDest();
    logger->addDest(m_logDest);

    QObject::connect(&m_timer, &QTimer::timeout, [this]() {
        // fail case
        if (m_logDest->isProblemDetected) {
            if (m_onResult) {
                m_onResult(false);
            }
            m_timer.stop();
        }

        // success case
        if (gsTestObj && gsTestObj->painted) {
            if (m_onResult) {
                m_onResult(true);
            }
            m_timer.stop();
        }
    });

    qmlRegisterType<GSTestObj>("Muse.Diagnostics", 1, 0, "GSTestObj");
}

GSProblemDetector::~GSProblemDetector()
{
    m_timer.stop();
    Logger* logger = Logger::instance();
    logger->removeDest(m_logDest);
    delete m_logDest;
}

QString GSProblemDetector::softwareMarkerFilePath() const
{
    GlobalConfiguration conf(modularity::globalCtx());
    return conf.userAppDataPath().toQString()
           + "/need_use_software_quick_backend_"
           + m_appVersion.toString()
           + ".dat";
}

void GSProblemDetector::setIsNeedUseSoftwareRender(bool arg)
{
    if (arg) {
        QFile file(softwareMarkerFilePath());
        file.open(QIODevice::WriteOnly); // create file
    } else {
        QFile::remove(softwareMarkerFilePath());
    }
}

bool GSProblemDetector::isNeedUseSoftwareRender() const
{
    return QFile::exists(softwareMarkerFilePath());
}

void GSProblemDetector::listen(const OnResult& f)
{
    m_onResult = f;
    m_timer.start(10);
}

void GSProblemDetector::destroy()
{
    GSProblemDetector* self = this;
    QTimer::singleShot(1, [self]() {
        delete self;
    });
}

// GSTestObj

GSTestObj::GSTestObj()
{
    setWidth(1);
    setHeight(1);

    GSProblemDetector::gsTestObj = this;
}

GSTestObj::~GSTestObj()
{
    GSProblemDetector::gsTestObj = nullptr;
}

void GSTestObj::paint(QPainter*)
{
    LOGD() << "GSTestObj painted";

    // just for test
    // LOGDA() << "Failed to build graphics pipeline state";

    painted = true;
}
