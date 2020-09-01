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
#include "staffsettingsmodel.h"

#include "log.h"

using namespace mu::instruments;
using namespace mu::notation;

StaffSettingsModel::StaffSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void StaffSettingsModel::load(const QVariant& staff)
{
    QVariantMap map = staff.toMap();

    m_staffIndex = map["staffIndex"].toInt();
    setIsSmallStaff(map["isSmall"].toBool());
    setCutawayEnabled(map["cutawayEnabled"].toBool());
    setStaffType(map["type"].toInt());

    m_voicesVisibility.clear();
    for (const QVariant& voice: map["voicesVisibility"].toList()) {
        m_voicesVisibility << voice.toBool();
    }

    emit voicesChanged();
}

QVariantList StaffSettingsModel::allStaffTypes() const
{
    QVariantList result;

    for (notation::StaffType type: notation::allStaffTypes()) {
        QVariantMap obj;

        obj["title"] = staffTypeToString(type);
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

QString StaffSettingsModel::staffType() const
{
    return staffTypeToString(m_type);
}

void StaffSettingsModel::setStaffType(int type)
{
    auto type_ = static_cast<notation::StaffType>(type);

    if (m_type == type_ || !parts()) {
        return;
    }

    m_type = type_;
    parts()->setStaffType(m_staffIndex, m_type);

    emit staffTypeChanged();
}

QVariantList StaffSettingsModel::voices() const
{
    QVariantList result;

    for (int i = 0; i < m_voicesVisibility.size(); ++i) {
        QVariantMap voice;

        voice["title"] = i + 1;
        voice["visible"] = m_voicesVisibility[i];

        result << voice;
    }

    return result;
}

void StaffSettingsModel::setVoiceVisible(int voiceIndex, bool visible)
{
    if (m_voicesVisibility[voiceIndex] == visible || !parts()) {
        return;
    }

    m_voicesVisibility[voiceIndex] = visible;
    parts()->setVoiceVisible(m_staffIndex, voiceIndex, visible);

    emit voicesChanged();
}

bool StaffSettingsModel::isSmallStaff() const
{
    return m_isSmallStaff;
}

void StaffSettingsModel::setIsSmallStaff(bool value)
{
    if (m_isSmallStaff == value || !parts()) {
        return;
    }

    m_isSmallStaff = value;
    parts()->setSmallStaff(m_staffIndex, value);

    emit isSmallStaffChanged();
}

bool StaffSettingsModel::cutawayEnabled() const
{
    return m_cutawayEnabled;
}

void StaffSettingsModel::setCutawayEnabled(bool value)
{
    if (m_cutawayEnabled == value || !parts()) {
        return;
    }

    m_cutawayEnabled = value;
    parts()->setCutaway(m_staffIndex, value);

    emit cutawayEnabledChanged();
}

void StaffSettingsModel::createLinkedStaff()
{
    if (parts()) {
        parts()->appendLinkedStaff(m_staffIndex);
    }
}

INotationParts* StaffSettingsModel::parts() const
{
    if (globalContext()->currentNotation()) {
        return globalContext()->currentNotation()->parts();
    }

    return nullptr;
}
