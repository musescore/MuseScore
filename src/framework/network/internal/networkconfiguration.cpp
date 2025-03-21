/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "networkconfiguration.h"

#include "global/configreader.h"

using namespace muse;
using namespace muse::network;

void NetworkConfiguration::init()
{
    m_userAgentConfig = ConfigReader::read(":/configs/user_agent.cfg");
}

RequestHeaders NetworkConfiguration::defaultHeaders(const std::string& userAgentName) const
{
    RequestHeaders headers;

    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = makeUserAgent(userAgentName);

    return headers;
}

QString NetworkConfiguration::makeUserAgent(const std::string& tplName) const
{
    std::string _tplName = tplName;
    if (_tplName.empty()) {
        static const std::string GLOBAL_USER_AGENT = "general";
        _tplName = GLOBAL_USER_AGENT;
    }

    QString osName;
#if defined(Q_OS_WIN)
    osName = "Windows";
#elif defined (Q_OS_MACOS)
    osName = "Mac";
#else
    osName = "Linux";
#endif

    const static QString kernelType = QSysInfo::kernelType();
    const static QString kernelVersion = QSysInfo::kernelVersion();
    const static QString osVersion = QSysInfo::productVersion();
    const static QString cpuArchitecture = QSysInfo::currentCpuArchitecture();

    QString result = m_userAgentConfig.value(_tplName).toQString();
    result = result.replace("{app_version}", application()->version().toString());
    result = result.replace("{app_build}", application()->build());

    result = result.replace("{kernel_type}", kernelType);
    result = result.replace("{kernel_version}", kernelVersion);
    result = result.replace("{os_name}", osName);
    result = result.replace("{os_version}", osVersion);
    result = result.replace("{cpu_arch}", cpuArchitecture);

    return result;
}
