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
#include "cloudconfiguration.h"

#include <QRandomGenerator>

#include "settings.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;

static const std::string module_name("cloud");
static const Settings::Key CLIENT_ID_KEY(module_name, "cloud/clientId");

static QByteArray generateClientId()
{
    QByteArray qtGeneratedId(QSysInfo::machineUniqueId());
    if (!qtGeneratedId.isEmpty()) {
        return qtGeneratedId;
    }

    QRandomGenerator* generator = QRandomGenerator::global();

    long long randId = generator->generate();
    constexpr size_t randBytes = sizeof(decltype(generator->generate()));

    for (size_t bytes = randBytes; bytes < sizeof(randId); bytes += randBytes) {
        randId <<= 8 * randBytes;
        randId += generator->generate();
    }

    return QString::number(randId, 16).toLatin1();
}

CloudConfiguration::CloudConfiguration(const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx)
{
}

void CloudConfiguration::init()
{
    if (settings()->value(CLIENT_ID_KEY).isNull()) {
        settings()->setSharedValue(CLIENT_ID_KEY, Val(generateClientId().toStdString()));
    }
}

std::string CloudConfiguration::clientId() const
{
    return settings()->value(CLIENT_ID_KEY).toString();
}

QByteArray CloudConfiguration::uploadingLicense() const
{
    return "all-rights-reserved";
}

io::path_t CloudConfiguration::tokensFilePath(const std::string& cloudName) const
{
    return globalConfiguration()->userAppDataPath() + "/" + cloudName + "_cred.dat";
}
