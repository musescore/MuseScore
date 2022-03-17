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

void StaffSettingsModel::load(const QString& staffId)
{
    if (!notationParts()) {
        return;
    }

    const Staff* staff = notationParts()->staff(staffId);
    if (!staff) {
        return;
    }

    m_staffId = staffId;
    m_config = notationParts()->staffConfig(m_staffId);
    m_type = staff->staffType()->type();
    auto presets = Ms::StaffType::presets();
    auto match = std::find_if(presets.begin(), presets.end(), [staff](const Ms::StaffType& t) {
        return t.isSameStructure(*staff->staffType());
    });
    if (match != presets.end()) {
        m_type = match->type();
    }

    m_voicesVisibility.clear();
    for (const QVariant& voice: staff->visibilityVoices()) {
        m_voicesVisibility << voice.toBool();
    }

    emit voicesChanged();
    emit staffTypeChanged();
    emit cutawayEnabledChanged();
    emit isSmallStaffChanged();
    emit allStaffTypesChanged();
    emit staffTypeChanged();
}

QVariantList StaffSettingsModel::allStaffTypes() const
{
    QVariantList result;
    auto staff = notationParts()->staff(m_staffId);
    if (staff == nullptr) {
        return result;
    }
    int maxLines = 0;
    bool isPercussion = false;
    if (staff->part() && staff->part()->instrument()) {
        if (staff->part()->instrument()->stringData()) {
            maxLines = staff->part()->instrument()->stringData()->frettedStrings();
        }
        isPercussion = staff->part()->instrument()->useDrumset();
    }

    for (const Ms::StaffType& type : Ms::StaffType::presets()) {
        if (isPercussion ? type.group() == Ms::StaffGroup::PERCUSSION
            : type.group() == Ms::StaffGroup::STANDARD || type.group() == Ms::StaffGroup::TAB && type.lines() <= maxLines) {
            QVariantMap obj;

            obj["text"] = staffTypeToString(type.type());
            obj["value"] = static_cast<int>(type.type());

            result << obj;
        }
    }

    return result;
}

int StaffSettingsModel::staffType() const
{
    return static_cast<int>(m_type);
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

    bool ok = notationParts()->setVoiceVisible(m_staffId, voiceIndex, visible);
    if (ok) {
        m_voicesVisibility[voiceIndex] = visible;
        emit voiceVisibilityChanged(voiceIndex, visible);
    }
}

bool StaffSettingsModel::isMainScore() const
{
    return currentNotation() == currentMasterNotation();
}

INotationPtr StaffSettingsModel::currentNotation() const
{
    return context()->currentNotation();
}

INotationPtr StaffSettingsModel::currentMasterNotation() const
{
    return context()->currentMasterNotation()->notation();
}

bool StaffSettingsModel::isSmallStaff() const
{
    return m_config.staffType.isSmall();
}

void StaffSettingsModel::setIsSmallStaff(bool value)
{
    if (m_config.staffType.isSmall() == value || !notationParts()) {
        return;
    }

    m_config.staffType.setSmall(value);
    notationParts()->setStaffConfig(m_staffId, m_config);

    emit isSmallStaffChanged();
}

bool StaffSettingsModel::cutawayEnabled() const
{
    return m_config.cutaway;
}

void StaffSettingsModel::setCutawayEnabled(bool value)
{
    if (m_config.cutaway == value || !notationParts()) {
        return;
    }

    m_config.cutaway = value;
    notationParts()->setStaffConfig(m_staffId, m_config);

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
    masterNotationParts()->appendLinkedStaff(linkedStaff, sourceStaff->id(), sourceStaff->part()->id());
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
