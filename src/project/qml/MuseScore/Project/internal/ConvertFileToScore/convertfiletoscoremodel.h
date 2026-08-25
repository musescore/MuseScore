/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"

#include "project/iprojectconfiguration.h"
#include "project/iconvertfiletoscorescenario.h"

namespace mu::project {
class ConvertFileToScoreModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(QString guidelinesLinkText READ guidelinesLinkText CONSTANT)
    Q_PROPERTY(QString accountAvatarUrl READ accountAvatarUrl CONSTANT)
    Q_PROPERTY(QString audioComUrl READ audioComUrl CONSTANT)

    Q_PROPERTY(QVariantList fileRequirements READ fileRequirements CONSTANT)

    QML_ELEMENT

    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<IConvertFileToScoreScenario> convertFileToScoreScenario = { this };
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::cloud::IAudioComService> audioComService = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;

public:
    explicit ConvertFileToScoreModel(QObject* parent = nullptr);

    QString guidelinesLinkText() const;
    QString accountAvatarUrl() const;
    QString audioComUrl() const;

    QVariantList fileRequirements() const;

    Q_INVOKABLE bool canSelectMultipleFiles(int type, const QStringList& paths) const;

    Q_INVOKABLE QStringList selectFiles(const QStringList& existingPaths = {});
    Q_INVOKABLE void validateFiles(const QStringList& pathsOrUrls);

    Q_INVOKABLE void confirmGoingBack();

signals:
    void validationFinished(int type, QVariantList paths);
    void goingBackConfirmed();
};
}
