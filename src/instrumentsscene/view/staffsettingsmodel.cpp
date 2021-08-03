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
#include "staffsettingsmodel.h"

using namespace mu::instrumentsscene;
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

        obj["text"] = staffTypeToString(type);
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
    auto type_ = static_cast<StaffType>(type);

    if (m_type == type_ || !notationParts()) {
        return;
    }

    m_type = type_;
    notationParts()->setStaffType(m_staffId, m_type);

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
    if (m_voicesVisibility[voiceIndex] == visible || !notationParts()) {
        return;
    }

    m_voicesVisibility[voiceIndex] = visible;
    notationParts()->setVoiceVisible(m_staffId, voiceIndex, visible);

    //! NOTE Do not send a signal to change the list
    //! This will lead to the re-creation of the controlы (checkboxes),
    //! and so we will lose the control with active focus,
    //! and new controls will be created and added.
    //! None of the controls will be the active focus.
    //! The checkbox state changes in the view.
    //! An alternative solution - we need to make a powerful model
    //! and not recreate elements when their state changes (do not reset it completely)
    //emit voicesChanged();
}

bool StaffSettingsModel::isSmallStaff() const
{
    return m_isSmallStaff;
}

void StaffSettingsModel::setIsSmallStaff(bool value)
{
    if (m_isSmallStaff == value || !notationParts()) {
        return;
    }

    m_isSmallStaff = value;
    notationParts()->setSmallStaff(m_staffId, value);

    emit isSmallStaffChanged();
}

bool StaffSettingsModel::cutawayEnabled() const
{
    return m_cutawayEnabled;
}

void StaffSettingsModel::setCutawayEnabled(bool value)
{
    if (m_cutawayEnabled == value || !notationParts()) {
        return;
    }

    m_cutawayEnabled = value;
    notationParts()->setCutawayEnabled(m_staffId, value);

    emit cutawayEnabledChanged();
}

void StaffSettingsModel::createLinkedStaff()
{
    if (!masterNotationParts()) {
        return;
    }

    const Staff* sourceStaff = masterNotationParts()->staff(m_staffId);
    if (!sourceStaff) {
        return;
    }

    Staff* linkedStaff = sourceStaff->clone();
    linkedStaff->setId(Staff::makeId());

    masterNotationParts()->appendStaff(linkedStaff, sourceStaff->part()->id());
    masterNotationParts()->linkStaves(sourceStaff->id(), linkedStaff->id());
}

INotationPartsPtr StaffSettingsModel::notationParts() const
{
    if (context()->currentNotation()) {
        return context()->currentNotation()->parts();
    }

    return nullptr;
}

INotationPartsPtr StaffSettingsModel::masterNotationParts() const
{
    if (context()->currentMasterNotation()) {
        return context()->currentMasterNotation()->parts();
    }

    return nullptr;
}
