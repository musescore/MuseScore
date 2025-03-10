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

void MuseSamplerActionController::init(const ReloadMuseSamplerFunc& reloadMuseSampler)
{
    m_reloadMuseSampler = reloadMuseSampler;

    dispatcher()->reg(this, "musesampler-check", this, &MuseSamplerActionController::checkLibraryIsDetected);
    dispatcher()->reg(this, "musesampler-reload", this, &MuseSamplerActionController::reloadMuseSampler);
}

void MuseSamplerActionController::checkLibraryIsDetected()
{
    std::string libVersion = museSamplerInfo()->version();
    std::string status;

    if (libVersion.empty()) {
        status = muse::trc("musesampler", "MuseSampler library is not found");
    } else {
        status = muse::qtrc("musesampler", "MuseSampler library is detected, version %1")
                 .arg(QString::fromStdString(libVersion)).toStdString();
    }

    interactive()->info(status, std::string());
}

void MuseSamplerActionController::reloadMuseSampler()
{
    IF_ASSERT_FAILED(m_reloadMuseSampler) {
        return;
    }

    if (!m_reloadMuseSampler()) {
        interactive()->error("", std::string("Could not reload MuseSampler library"));
    }
}
