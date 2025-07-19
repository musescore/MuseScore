/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef PROJECTPROPERTIESMODEL_H
#define PROJECTPROPERTIESMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"

namespace mu::project {
class ProjectPropertiesModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(muse::IInteractive, interactive)

    Q_PROPERTY(QString filePath READ filePath CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString revision READ revision CONSTANT)
    Q_PROPERTY(QString apiLevel READ apiLevel CONSTANT)

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

signals:
    void propertyAdded(int index);

private:
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
};
}

#endif // PROJECTPROPERTIESMODEL_H
