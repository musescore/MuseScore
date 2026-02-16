/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "imusesoundsconfiguration.h"

namespace mu::musesounds {
class MuseSoundsDevToolsModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool enableTestMode READ enableTestMode WRITE setEnableTestMode NOTIFY enableTestModeChanged)
    Q_PROPERTY(int version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString heroImagePath READ heroImagePath WRITE setHeroImagePath NOTIFY heroImagePathChanged)
    Q_PROPERTY(QString updateNotes READ updateNotes WRITE setUpdateNotes NOTIFY updateNotesChanged)
    Q_PROPERTY(QString ctaLink READ ctaLink WRITE setCtaLink NOTIFY ctaLinkChanged)
    Q_PROPERTY(QString selectedLanguage READ selectedLanguage WRITE setSelectedLanguage NOTIFY selectedLanguageChanged)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)

    Q_PROPERTY(
        QString currentUpdateData READ currentUpdateData_property WRITE setCurrentUpdateData_property NOTIFY currentUpdateDataChanged FINAL)

    muse::GlobalInject<IMuseSoundsConfiguration> configuration;
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit MuseSoundsDevToolsModel(QObject* parent = nullptr);

    Q_INVOKABLE void applyUpdateData();
    Q_INVOKABLE void openUpdateDialog();

    bool enableTestMode() const;
    void setEnableTestMode(bool enabled);

    int version() const;
    void setVersion(int version);

    QString heroImagePath() const;
    void setHeroImagePath(const QString& path);

    QString updateNotes() const;
    void setUpdateNotes(const QString& notes);

    QString ctaLink() const;
    void setCtaLink(const QString& link);

    QString selectedLanguage() const;
    void setSelectedLanguage(const QString& lang);

    QStringList availableLanguages() const;

    QString currentUpdateData_property() const;
    void setCurrentUpdateData_property(const QString& data);
    void setCurrentUpdateData(const QJsonDocument& data);

private:
    QString heroImageToBase64() const;
    QJsonObject processUpdateNotes() const;
    QJsonObject processCTALink() const;

    void updateAvailableLanguages();

signals:
    void enableTestModeChanged();
    void versionChanged();
    void heroImagePathChanged();
    void updateNotesChanged();
    void ctaLinkChanged();
    void selectedLanguageChanged();
    void availableLanguagesChanged();

    void currentUpdateDataChanged();

private:
    int m_version = 0;
    QString m_heroImagePath;
    QString m_updateNotes;
    QString m_ctaLink;
    QString m_selectedLanguage;
    QStringList m_availableLanguages;
    QJsonDocument m_currentUpdateData;
};
}
