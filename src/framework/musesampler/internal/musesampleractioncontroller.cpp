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

    std::string libVersion = resolver->version();
    std::string status;

    if (libVersion.empty()) {
        status = muse::trc("musesampler", "MuseSampler library is not found");
    } else {
        if (configuration()->shouldShowBuildNumber()) {
            libVersion += "." + std::to_string(resolver->buildNumber());
        }

        status = muse::qtrc("musesampler", "MuseSampler library is detected, version %1")
                 .arg(QString::fromStdString(libVersion)).toStdString();
    }

    interactive()->info(status, std::string());
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
