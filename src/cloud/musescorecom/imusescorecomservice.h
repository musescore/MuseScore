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
#ifndef MU_CLOUD_IMUSESCORECOMSERVICE_H
#define MU_CLOUD_IMUSESCORECOMSERVICE_H

#include <QUrl>

#include "modularity/imoduleinterface.h"
#include "progress.h"

#include "cloud/cloudtypes.h"
#include "cloud/iauthorizationservice.h"

class QIODevice;
class QString;

namespace mu::cloud {
class IMuseScoreComService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMuseScoreComService)

public:
    virtual ~IMuseScoreComService() = default;

    virtual IAuthorizationServicePtr authorization() = 0;

    virtual QUrl scoreManagerUrl() const = 0;

    virtual framework::ProgressPtr uploadScore(QIODevice& scoreData, const QString& title,
                                               cloud::Visibility visibility = cloud::Visibility::Private,
                                               const QUrl& sourceUrl = QUrl(), int revisionId = 0) = 0;
    virtual framework::ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl) = 0;

    virtual RetVal<cloud::ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) = 0;
};
}

#endif // MU_CLOUD_IMUSESCORECOMSERVICE_H
