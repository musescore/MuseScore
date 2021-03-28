//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "exportscoresettingsmodel.h"

#include <QRegularExpression>

using namespace mu::userscores;
using namespace mu::framework;

ExportScoreSettingsModel::ExportScoreSettingsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void ExportScoreSettingsModel::load(QString suffix)
{
    beginResetModel();

    for (auto key : m_keys) {
        settings()->valueChanged(key).resetOnReceive(this);
    }

    if (suffix == "pdf") {
        m_keys = pdfKeys();
    } else if (suffix == "png") {
        m_keys = pngKeys();
    } else if (suffix == "mp3") {
        m_keys = mp3Keys();
    } else if (suffix == "wav" || suffix == "ogg" || suffix == "flac") {
        m_keys = audioKeys();
    } else if (suffix == "midi") {
        m_keys = midiKeys();
    } else if (suffix == "musicxml") {
        m_keys = xmlKeys();
    } else {
        m_keys.clear();
    }

    for (int i = 0; i < m_keys.size(); i++) {
        settings()->valueChanged(m_keys[i]).onReceive(this, [this, i](const Val&) {
            emit dataChanged(index(i), index(i));
        });
    }

    endResetModel();
}

QVariant ExportScoreSettingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    auto key = m_keys[index.row()];
    auto item = settings()->findItem(key);
    auto info = item.info;

    switch (role) {
    case LabelRole:
        return info ? info->translatedLabel() : QString::fromStdString(key.key).replace(QRegularExpression("^export/"), "");
    case TypeRole:
        return typeToString(item);
    case ValRole:
        return settings()->value(item.key).toQVariant();
    case InfoRole:
        return info ? info->toQVariant() : QVariant();
    }

    return QVariant();
}

int ExportScoreSettingsModel::rowCount(const QModelIndex&) const
{
    return m_keys.size();
}

QHash<int, QByteArray> ExportScoreSettingsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { LabelRole, "labelRole" },
        { TypeRole, "typeRole" },
        { ValRole, "valRole" },
        { InfoRole, "infoRole" }
    };

    return roles;
}

void ExportScoreSettingsModel::setValue(int idx, QVariant newVal)
{
    settings()->setValue(m_keys[idx], Val::fromQVariant(newVal));
}

QString ExportScoreSettingsModel::typeToString(const Settings::Item& item) const
{
    if (!item.info || item.info->type() == SettingsInfo::Auto) {
        switch (item.value.type()) {
        case Val::Type::Bool: return "Bool";
        case Val::Type::Int: return "Int";
        case Val::Type::Double: return "Double";
        case Val::Type::String: return "String";
        case Val::Type::Color: return "Color";
        default: return "Undefined";
        }
    }

    switch (item.info->type()) {
    case SettingsInfo::NumberSpinner: return "NumberSpinner";
    case SettingsInfo::RadioButtonGroup: return "RadioButtonGroup";
    case SettingsInfo::ComboBox: return "ComboBox";
    default: return "Undefined";
    }
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::pdfKeys() const
{
    KeyList keys {
        Settings::Key("iex_imagesexport", "export/pdf/dpi")
    };

    return keys;
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::pngKeys() const
{
    KeyList keys {
        Settings::Key("iex_imagesexport", "export/png/resolution"),
        Settings::Key("iex_imagesexport", "export/png/useTransparency")
    };

    return keys;
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::mp3Keys() const
{
    NOT_IMPLEMENTED;

    KeyList keys = audioKeys();
    //keys << Settings::Key(); //! TODO: Bit rate key

    return keys;
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::audioKeys() const
{
    NOT_IMPLEMENTED;

    KeyList keys {
        //! TODO: Normalize key
        //! TODO: Sample rate key
    };

    return keys;
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::midiKeys() const
{
    NOT_IMPLEMENTED;

    KeyList keys {
        //! TODO: Expand Repeats key
        //! TODO: Export RPNs key
    };

    return keys;
}

ExportScoreSettingsModel::KeyList ExportScoreSettingsModel::xmlKeys() const
{
    NOT_IMPLEMENTED;

    KeyList keys {
        //! TODO: MusicXML Layout key
    };

    return keys;
}
