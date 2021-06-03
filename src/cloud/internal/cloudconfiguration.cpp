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

using namespace mu::cloud;

void CloudConfiguration::init()
{
    fileSystem()->makePath(tokensFilePath());
}

QUrl CloudConfiguration::authorizationUrl() const
{
    return QUrl("https://musescore.com/user/auth/oauth2server");
}

QUrl CloudConfiguration::accessTokenUrl() const
{
    return QUrl("https://musescore.com/user/auth/oauth2server/token");
}

QUrl CloudConfiguration::userInfoApiUrl() const
{
    return apiUrl() + "/user/me";
}

mu::io::path CloudConfiguration::tokensFilePath() const
{
    return globalConfiguration()->dataPath() + "/cred.dat";
}

QString CloudConfiguration::apiUrl() const
{
    return "https://api.musescore.com/v2";
}
