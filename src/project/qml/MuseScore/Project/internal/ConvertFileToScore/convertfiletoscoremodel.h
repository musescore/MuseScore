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

    Q_PROPERTY(QString accountAvatarUrl READ accountAvatarUrl CONSTANT)
    Q_PROPERTY(QString guidelinesUrl READ guidelinesUrl CONSTANT)

    Q_PROPERTY(int convertType READ convertType NOTIFY convertTypeChanged)
    Q_PROPERTY(QStringList selectedPaths READ selectedPaths WRITE setSelectedPaths NOTIFY selectedPathsChanged)
    Q_PROPERTY(QString selectedLink READ selectedLink WRITE setSelectedLink NOTIFY selectedLinkChanged)

    Q_PROPERTY(QVariantList fileRequirements READ fileRequirements NOTIFY fileRequirementsChanged)
    Q_PROPERTY(QVariantMap convertLimits READ convertLimits NOTIFY convertTypeChanged)
    Q_PROPERTY(bool canSelectMultipleFiles READ canSelectMultipleFiles NOTIFY convertTypeChanged)

    Q_PROPERTY(QString linkHintText READ linkHintText CONSTANT)
    Q_PROPERTY(QString linkHintPlainText READ linkHintPlainText CONSTANT)
    Q_PROPERTY(int maxLinkLength READ maxLinkLength CONSTANT)

    QML_ELEMENT

    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<IConvertFileToScoreScenario> convertFileToScoreScenario = { this };
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::cloud::IAudioComService> audioComService = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;

public:
    explicit ConvertFileToScoreModel(QObject* parent = nullptr);

    QString accountAvatarUrl() const;
    QString guidelinesUrl() const;

    int convertType() const; // OMR = 0, Audio2Score = 1

    QStringList selectedPaths() const;
    void setSelectedPaths(const QStringList& paths);

    QString selectedLink() const;
    void setSelectedLink(const QString& link);

    QVariantList fileRequirements() const;
    QVariantMap convertLimits() const;
    bool canSelectMultipleFiles() const;

    QString linkHintText() const;
    QString linkHintPlainText() const;
    int maxLinkLength() const;

    Q_INVOKABLE void validateFiles(const QStringList& pathsOrUrls);
    Q_INVOKABLE void selectAndValidateFiles(const QStringList& existingPaths = {});

    Q_INVOKABLE void confirmGoingBack();

signals:
    void convertTypeChanged();
    void selectedPathsChanged();
    void selectedLinkChanged();
    void fileRequirementsChanged();

    void validationFinished();
    void goingBackConfirmed();

private:
    void setConvertType(int type);
    FileCategory selectedFileCategory() const;

    QStringList selectFiles(const QStringList& existingPaths = {});

    ConvertType m_convertType = ConvertType::Omr;
    QStringList m_selectedPaths;
    QString m_selectedLink;
};
}
