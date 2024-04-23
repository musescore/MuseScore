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
#include "learnconfiguration.h"
#include "global/configreader.h"

#include "settings.h"

using namespace muse;
using namespace muse::learn;
using namespace muse::network;

static const std::string module_name("learn");
static const Settings::Key GET_PLAYLISTS_TESTING_MODE_KEY(module_name, "learn/getPlaylistsTestingMode");

void LearnConfiguration::init()
{
    m_config = ConfigReader::read(":/configs/learn.cfg");

    settings()->setDefaultValue(GET_PLAYLISTS_TESTING_MODE_KEY, Val(false));
}

RequestHeaders LearnConfiguration::headers() const
{
    RequestHeaders headers;
    headers.rawHeaders["Accept"] = "application/json";
    return headers;
}

QUrl LearnConfiguration::startedPlaylistUrl() const
{
    return !isTestingMode() ? QUrl(m_config.value("started_playlist_url").toQString())
           : QUrl(m_config.value("started_playlist_url.test").toQString());
}

QUrl LearnConfiguration::advancedPlaylistUrl() const
{
    return !isTestingMode() ? QUrl(m_config.value("advanced_playlist_url").toQString())
           : QUrl(m_config.value("advanced_playlist_url.test").toQString());
}

bool LearnConfiguration::classesEnabled() const
{
    return m_config.value("classes_enabled", Val(true)).toBool();
}

bool LearnConfiguration::isTestingMode() const
{
    return settings()->value(GET_PLAYLISTS_TESTING_MODE_KEY).toBool();
}
