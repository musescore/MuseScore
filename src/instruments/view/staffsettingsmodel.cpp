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

using namespace mu::instruments;
using namespace mu::notation;

StaffSettingsModel::StaffSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void StaffSettingsModel::load(const QVariant& staff)
{
    QVariantMap map = staff.toMap();

    m_staffId = map["staffId"].toString();
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
    parts()->setStaffType(m_staffId, m_type);

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
    parts()->setVoiceVisible(m_staffId, voiceIndex, visible);

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
    parts()->setSmallStaff(m_staffId, value);

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
    parts()->setCutawayEnabled(m_staffId, value);

    emit cutawayEnabledChanged();
}

void StaffSettingsModel::createLinkedStaff()
{
    if (!parts()) {
        return;
    }

    Staff* sourceStaff = staff();
    if (!sourceStaff) {
        return;
    }

    Staff* linkedStaff = sourceStaff->clone();
    linkedStaff->setId(Staff::makeId());

    parts()->appendStaff(linkedStaff, sourceStaff->part()->id());
    parts()->cloneStaff(sourceStaff->id(), linkedStaff->id());
}

Staff* StaffSettingsModel::staff() const
{
    for (const Part* part : parts()->partList()) {
        for (Staff* staff : *part->staves()) {
            if (staff->id() == m_staffId) {
                return staff;
            }
        }
    }

    return nullptr;
}

INotationPartsPtr StaffSettingsModel::parts() const
{
    if (context()->currentMasterNotation()) {
        return context()->currentMasterNotation()->parts();
    }

    return nullptr;
}
