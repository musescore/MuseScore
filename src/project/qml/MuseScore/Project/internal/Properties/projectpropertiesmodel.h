/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QList>
#include <QString>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "interactive/iplatforminteractive.h"
#include "project/iprojectconfiguration.h"
#include "project/iprojectsession.h"

#include "types/projectmeta.h"

namespace mu::project {
class ProjectPropertiesModel : public QAbstractListModel, public QQmlParserStatus, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString filePath READ filePath CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString revision READ revision CONSTANT)
    Q_PROPERTY(QString apiLevel READ apiLevel CONSTANT)

    QML_ELEMENT

    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<project::IProjectSession> context = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;

public:
    explicit ProjectPropertiesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filePath() const;
    QString version() const;
    QString revision() const;
    QString apiLevel() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void newProperty();
    Q_INVOKABLE void deleteProperty(int index);
    Q_INVOKABLE void saveProperties();
    Q_INVOKABLE void openFileLocation();

    //! The languages that the score can be in, as in the New Score dialog (see NewScoreModel),
    //! including the language of the interface, which an existing score may be in
    Q_INVOKABLE QVariantList instrumentNamesLanguages() const;

    //! The language of the score: its code, or empty for the language of the interface
    Q_INVOKABLE QString instrumentNamesLanguage() const;

    //! Changes the language of the score. It is saved by saveProperties(). Changing it does not change the
    //! instruments that are already in the score, only the instruments that are added later and the palettes.
    Q_INVOKABLE void setInstrumentNamesLanguage(const QString& languageCode);

signals:
    void propertyAdded(int index);

private:
    void classBegin() override;
    void componentComplete() override {}
    void init();

    enum Roles {
        PropertyName = Qt::UserRole + 1,
        PropertyValue,
        IsStandardProperty,
        IsMultiLineEdit
    };

    struct Property {
        QString key, name, value;
        bool isStandardProperty = false;
        bool isMultiLineEdit = false;
    };

    project::ProjectMeta m_projectMetaInfo;
    QList<Property> m_properties;

    // The meta tag of the language is not shown as a property, but with a dropdown
    QString m_textLanguage;
};
}
