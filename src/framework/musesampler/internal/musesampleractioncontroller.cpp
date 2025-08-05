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
#include "musesampleractioncontroller.h"

#include "translation.h"
#include "log.h"

using namespace muse::musesampler;

void MuseSamplerActionController::init(std::weak_ptr<MuseSamplerResolver> resolver)
{
    m_museSamplerResolver = resolver;

    dispatcher()->reg(this, "musesampler-check", this, &MuseSamplerActionController::checkLibraryIsDetected);
    dispatcher()->reg(this, "musesampler-reload", this, &MuseSamplerActionController::reloadMuseSampler);
    dispatcher()->reg(this, "process-online-sounds", this, &MuseSamplerActionController::processOnlineSounds);
}

void MuseSamplerActionController::checkLibraryIsDetected()
{
    std::shared_ptr<MuseSamplerResolver> resolver = m_museSamplerResolver.lock();
    if (!resolver) {
        return;
    }

    const Version& libVersion = resolver->version();
    String libVersionStr = libVersion.toString();

    if (configuration()->shouldShowBuildNumber() && resolver->buildNumber() >= 0) {
        libVersionStr += u"." + String::number(resolver->buildNumber());
    }

    String status;

    if (resolver->isLoaded()) {
        status = muse::mtrc("musesampler", "MuseSampler library is detected, version %1")
                 .arg(libVersionStr);
    } else if (!libVersion.isNull() && libVersion < configuration()->minSupportedVersion()) {
        status = muse::mtrc("musesampler", "Installed MuseSampler library is not supported, version %1")
                 .arg(libVersionStr);
    } else {
        status = muse::mtrc("musesampler", "MuseSampler library is not found");
    }

    interactive()->info(status.toStdString(), std::string());
}

void MuseSamplerActionController::reloadMuseSampler()
{
    std::shared_ptr<MuseSamplerResolver> resolver = m_museSamplerResolver.lock();
    if (!resolver) {
        return;
    }

    if (resolver->reloadAllInstruments()) {
        interactive()->error("", std::string("Could not reload MuseSampler library"));
    }
}

void MuseSamplerActionController::processOnlineSounds()
{
    std::shared_ptr<MuseSamplerResolver> resolver = m_museSamplerResolver.lock();
    if (!resolver) {
        return;
    }

    resolver->processOnlineSounds();
}
