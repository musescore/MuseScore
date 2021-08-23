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

    INJECT(instruments, context::IGlobalContext, context)

    Q_PROPERTY(QString staffType READ staffType NOTIFY staffTypeChanged)
    Q_PROPERTY(QVariantList voices READ voices NOTIFY voicesChanged)
    Q_PROPERTY(bool isSmallStaff READ isSmallStaff NOTIFY isSmallStaffChanged)
    Q_PROPERTY(bool cutawayEnabled READ cutawayEnabled NOTIFY cutawayEnabledChanged)

    Q_PROPERTY(bool isMainScore READ isMainScore NOTIFY isMainScoreChanged)

public:
    explicit StaffSettingsModel(QObject* parent = nullptr);

    QString staffType() const;
    QVariantList voices() const;
    bool isSmallStaff() const;
    bool cutawayEnabled() const;

    Q_INVOKABLE void load(const QString& staffId);

    Q_INVOKABLE QVariantList allStaffTypes() const;
    Q_INVOKABLE void createLinkedStaff();

    Q_INVOKABLE void setStaffType(int type);
    Q_INVOKABLE void setIsSmallStaff(bool value);
    Q_INVOKABLE void setCutawayEnabled(bool value);
    Q_INVOKABLE void setVoiceVisible(int voiceIndex, bool visible);

    bool isMainScore() const;

signals:
    void staffTypeChanged();
    void voicesChanged();
    void voiceVisibilityChanged(int voiceIndex, bool visible);
    void isSmallStaffChanged();
    void cutawayEnabledChanged();

    void isMainScoreChanged(bool isMainScore);

private:
    notation::INotationPtr currentNotation() const;
    notation::INotationPtr currentMasterNotation() const;
    notation::INotationPartsPtr notationParts() const;
    notation::INotationPartsPtr masterNotationParts() const;

    ID m_staffId;
    QList<bool> m_voicesVisibility;
    notation::StaffType m_type = notation::StaffType::STANDARD;
    notation::StaffConfig m_config;
};
}

#endif // MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H
