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
#ifndef MU_CLOUD_IAUDIOCOMSERVICE_H
#define MU_CLOUD_IAUDIOCOMSERVICE_H

#include "modularity/imoduleinterface.h"
#include "progress.h"

#include "cloud/cloudtypes.h"

class QIODevice;
class QString;

namespace mu::cloud {
class IAudioComService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioComService)

public:
    virtual ~IAudioComService() = default;

    virtual IAuthorizationServicePtr authorization() = 0;

    virtual framework::ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QString& title,
                                               const QUrl& existingUrl, Visibility visibility = Visibility::Private,
                                               bool replaceExisting = false) = 0;

    virtual CloudInfo cloudInfo() const = 0;
};
}

#endif // MU_CLOUD_IAUDIOCOMSERVICE_H
