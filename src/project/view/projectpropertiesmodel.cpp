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
#include "projectpropertiesmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::modularity;
using namespace mu::notation;
using namespace mu::project;

static const QString WORK_TITLE_TAG("workTitle"); //// Keep updated with notationproject.cpp
static const QString COMPOSER_TAG("composer");
static const QString LYRICIST_TAG("lyricist");
static const QString SOURCE_TAG("source");
static const QString AUDIO_COM_URL_TAG("audioComFile");
static const QString COPYRIGHT_TAG("copyright");
static const QString TRANSLATOR_TAG("translator");
static const QString ARRANGER_TAG("arranger");
static const QString CREATION_DATE_TAG("creationDate");
static const QString PLATFORM_TAG("platform");

ProjectPropertiesModel::ProjectPropertiesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    INotationProjectPtr project = context()->currentProject();
    if (project) {
        m_projectMetaInfo = project->metaInfo();
    }
}

void ProjectPropertiesModel::load()
{
    beginResetModel();

    m_properties = {
        { WORK_TITLE_TAG,          qtrc("project", "Work title"),         m_projectMetaInfo.title,                      true },
        { ARRANGER_TAG,            qtrc("project", "Arranger"),           m_projectMetaInfo.arranger,                   true },
        { COMPOSER_TAG,            qtrc("project", "Composer"),           m_projectMetaInfo.composer,                   true },
        { COPYRIGHT_TAG,           qtrc("project", "Copyright"),          m_projectMetaInfo.copyright,                  true },
        { CREATION_DATE_TAG,       qtrc("project", "Creation date"),      m_projectMetaInfo.creationDate.toString(),    true },
        { LYRICIST_TAG,            qtrc("project", "Lyricist"),           m_projectMetaInfo.lyricist,                   true },
        { TRANSLATOR_TAG,          qtrc("project", "Translator"),         m_projectMetaInfo.translator,                 true },
        { PLATFORM_TAG,            qtrc("project", "Platform"),           m_projectMetaInfo.platform,                   true },
        { SOURCE_TAG,              qtrc("project", "Source"),             m_projectMetaInfo.source,                     true },
        { AUDIO_COM_URL_TAG,       qtrc("project", "Audio.com URL"),      m_projectMetaInfo.audioComUrl,                true }
    };

    QVariantMap additionalProperties = m_projectMetaInfo.additionalTags;
    for (const QString& propertyName : additionalProperties.keys()) {
        m_properties.append({ "", propertyName, additionalProperties[propertyName].toString() });
    }

    endResetModel();
}

QVariant ProjectPropertiesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Property& property = m_properties[index.row()];

    switch (role) {
    case PropertyName:
        return property.name;
    case PropertyValue:
        return property.value;
    case IsStandardProperty:
        return property.isStandardProperty;
    }

    return QVariant();
}

bool ProjectPropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    Property& property = m_properties[index.row()];

    switch (role) {
    case PropertyName:
        if (!value.canConvert<QString>()) {
            return false;
        }
        property.name = value.toString();
        return true;
    case PropertyValue:
        if (!value.canConvert<QString>()) {
            return false;
        }
        property.value = value.toString();
        return true;
    default:
        return false;
    }
}

int ProjectPropertiesModel::rowCount(const QModelIndex&) const
{
    return m_properties.size();
}

QHash<int, QByteArray> ProjectPropertiesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { PropertyName, "propertyName" },
        { PropertyValue, "propertyValue" },
        { IsStandardProperty, "isStandardProperty" }
    };

    return roles;
}

QString ProjectPropertiesModel::filePath() const
{
    return m_projectMetaInfo.filePath.toQString();
}

QString ProjectPropertiesModel::version() const
{
    return m_projectMetaInfo.musescoreVersion;
}

QString ProjectPropertiesModel::revision() const
{
    int rev = m_projectMetaInfo.musescoreRevision;
    QString revision;
    if (rev > 99999) {     // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
        revision = QString::number(rev, 16);
    } else {
        revision = QString::number(rev, 10);
    }

    return revision;
}

QString ProjectPropertiesModel::apiLevel() const
{
    return QString::number(m_projectMetaInfo.mscVersion);
}

void ProjectPropertiesModel::newProperty()
{
    int destinationIndex = m_properties.size();
    beginInsertRows(QModelIndex(), destinationIndex, destinationIndex);

    Property property = { "", "", "" };
    m_properties.append(property);

    endInsertRows();

    emit propertyAdded(destinationIndex);
}

void ProjectPropertiesModel::deleteProperty(int index)
{
    if (index < 0 && index >= m_properties.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);

    m_properties.removeAt(index);

    endRemoveRows();
}

void ProjectPropertiesModel::saveProperties()
{
    INotationProjectPtr project = context()->currentProject();
    if (!project) {
        return;
    }

    ProjectMeta meta = project->metaInfo();
    meta.additionalTags = {};

    for (const Property& property : m_properties) {
        if (property.key == WORK_TITLE_TAG) {
            meta.title = property.value;
        } else if (property.key == ARRANGER_TAG) {
            meta.arranger = property.value;
        } else if (property.key == COMPOSER_TAG) {
            meta.composer = property.value;
        } else if (property.key == COPYRIGHT_TAG) {
            meta.copyright = property.value;
        } else if (property.key == CREATION_DATE_TAG) {
            meta.creationDate = QDate::fromString(property.value);
        } else if (property.key == LYRICIST_TAG) {
            meta.lyricist = property.value;
        } else if (property.key == TRANSLATOR_TAG) {
            meta.translator = property.value;
        } else if (property.key == PLATFORM_TAG) {
            meta.platform = property.value;
        } else if (property.key == SOURCE_TAG) {
            meta.source = property.value;
        } else if (property.key == AUDIO_COM_URL_TAG) {
            meta.audioComUrl = property.value;
        } else {
            if (!property.name.isEmpty() && !property.value.isEmpty()) {
                meta.additionalTags[property.name] = property.value;
            }
        }
    }

    project->setMetaInfo(meta, true);
}

void ProjectPropertiesModel::openFileLocation()
{
    Ret ret = interactive()->revealInFileBrowser(m_projectMetaInfo.filePath.toQString());

    if (!ret) {
        LOGE() << "Could not open folder: " << m_projectMetaInfo.filePath.toQString();
    }
}
