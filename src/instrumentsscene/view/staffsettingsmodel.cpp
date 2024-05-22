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
#include "staffsettingsmodel.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

StaffSettingsModel::StaffSettingsModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
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

    m_voicesVisibility.clear();
    for (const bool& voice : staff->visibilityVoices()) {
        m_voicesVisibility << voice;
    }

    emit cutawayEnabledChanged();
    emit isSmallStaffChanged();
    emit voicesChanged();
    emit allStaffTypesChanged();
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

QVariantList StaffSettingsModel::allStaffTypes() const
{
    QVariantList result;

    const Staff* staff = notationParts()->staff(m_staffId);
    if (!staff) {
        return result;
    }

    const Part* part = staff->part();
    if (!part) {
        return result;
    }

    int maxLines = 0;
    bool isPercussion = false;

    if (const Instrument* instrument = part->instrument()) {
        if (const StringData* stringData = instrument->stringData()) {
            maxLines = stringData->frettedStrings();
        }

        isPercussion = instrument->useDrumset();
    }

    auto isTypeAllowed = [maxLines, isPercussion](const mu::engraving::StaffType& type) {
        switch (type.group()) {
        case mu::engraving::StaffGroup::PERCUSSION: return isPercussion;
        case mu::engraving::StaffGroup::TAB: return type.lines() <= maxLines;
        case mu::engraving::StaffGroup::STANDARD: return true;
        }

        return false;
    };

    for (const mu::engraving::StaffType& type : mu::engraving::StaffType::presets()) {
        if (isTypeAllowed(type)) {
            QVariantMap obj;

            obj["text"] = staffTypeToString(type.type());
            obj["value"] = static_cast<int>(type.type());

            result << obj;
        }
    }

    return result;
}

bool StaffSettingsModel::isMainScore() const
{
    return currentNotation() == currentMasterNotation();
}

int StaffSettingsModel::staffType() const
{
    return static_cast<int>(m_config.staffType.type());
}

void StaffSettingsModel::setStaffType(int type)
{
    auto type_ = static_cast<StaffTypeId>(type);

    if (m_config.staffType.type() == type_ || !notationParts()) {
        return;
    }

    bool wasSmall = m_config.staffType.isSmall();

    notationParts()->setStaffType(m_staffId, type_);
    m_config = notationParts()->staffConfig(m_staffId);

    if (wasSmall != m_config.staffType.isSmall()) {
        emit isSmallStaffChanged();
    }

    emit staffTypeChanged();
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
    if (!masterNotationParts()->appendLinkedStaff(linkedStaff, sourceStaff->id(), sourceStaff->part()->id())) {
        linkedStaff->unlink();
        delete linkedStaff;
    }
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
