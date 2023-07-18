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
#ifndef MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H
#define MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"

namespace mu::instrumentsscene {
class StaffSettingsModel : public QObject
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)

    Q_PROPERTY(int staffType READ staffType WRITE setStaffType NOTIFY staffTypeChanged)
    Q_PROPERTY(bool isSmallStaff READ isSmallStaff WRITE setIsSmallStaff NOTIFY isSmallStaffChanged)
    Q_PROPERTY(bool cutawayEnabled READ cutawayEnabled WRITE setCutawayEnabled NOTIFY cutawayEnabledChanged)

    Q_PROPERTY(QVariantList voices READ voices NOTIFY voicesChanged)
    Q_PROPERTY(QVariantList allStaffTypes READ allStaffTypes NOTIFY allStaffTypesChanged)

    Q_PROPERTY(bool isMainScore READ isMainScore NOTIFY isMainScoreChanged)

public:
    explicit StaffSettingsModel(QObject* parent = nullptr);

    int staffType() const;
    bool isSmallStaff() const;
    bool cutawayEnabled() const;

    QVariantList voices() const;
    QVariantList allStaffTypes() const;

    bool isMainScore() const;

    Q_INVOKABLE void load(const QString& staffId);

    Q_INVOKABLE void createLinkedStaff();
    Q_INVOKABLE void setVoiceVisible(int voiceIndex, bool visible);

public slots:
    void setStaffType(int type);
    void setIsSmallStaff(bool value);
    void setCutawayEnabled(bool value);

signals:
    void staffTypeChanged();
    void voicesChanged();
    void voiceVisibilityChanged(int voiceIndex, bool visible);
    void isSmallStaffChanged();
    void cutawayEnabledChanged();
    void allStaffTypesChanged();

    void isMainScoreChanged(bool isMainScore);

private:
    notation::INotationPtr currentNotation() const;
    notation::INotationPtr currentMasterNotation() const;
    notation::INotationPartsPtr notationParts() const;
    notation::INotationPartsPtr masterNotationParts() const;

    ID m_staffId;
    QList<bool> m_voicesVisibility;
    notation::StaffTypeId m_type = notation::StaffTypeId::STANDARD;
    notation::StaffConfig m_config;
};
}

#endif // MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H
