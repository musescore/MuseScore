//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "devtools/devtoolsmodule.h"

#include "modularity/ioc.h"

#include "log.h"
#include "thirdparty/haw_logger/logger/logdefdest.h"

#include "global/iglobalconfiguration.h"

#include "dump/crashhandler.h"

using namespace mu::framework;
using namespace mu::devtools;

static CrashHandler* s_crashHandler;

std::string DevToolsModule::moduleName() const
{
    return "devtools";
}

//! NOTE For dump testing
//static void crash() { volatile int* a = (int*)(NULL); *a = 1; }

void DevToolsModule::onInit()
{
    auto globalConf = ioc()->resolve<IGlobalConfiguration>(moduleName());
    IF_ASSERT_FAILED(globalConf) {
        return;
    }

    //! Log ---
    using namespace haw::logger;
    Logger* logger = Logger::instance();
    logger->clearDests();

    //! Console
    logger->addDest(new ConsoleLogDest(LogLayout("${time} | ${type|5} | ${thread} | ${tag|10} | ${message}")));

    //! File, this creates a file named "data/logs/MuseScore_yyMMdd.log"
    std::string logsPath = globalConf->logsPath().c_str();
    LOGI() << "logs path: " << logsPath;
    logger->addDest(new FileLogDest(logsPath, "MuseScore", "log",
                                    LogLayout("${datetime} | ${type|5} | ${thread} | ${tag|10} | ${message}")));

#ifndef NDEBUG
    logger->setLevel(haw::logger::Debug);
#else
    logger->setLevel(haw::logger::Normal);
#endif

    //! Dump ---
    s_crashHandler = new CrashHandler(globalConf->logsPath().toStdString());

    //! NOTE For dump testing
    //crash();
}
