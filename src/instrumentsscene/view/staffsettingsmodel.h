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
#ifndef MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H
#define MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"

namespace mu::instrumentsscene {
class StaffSettingsModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(int staffType READ staffType WRITE setStaffType NOTIFY staffTypeChanged)
    Q_PROPERTY(bool isSmallStaff READ isSmallStaff WRITE setIsSmallStaff NOTIFY isSmallStaffChanged)
    Q_PROPERTY(bool cutawayEnabled READ cutawayEnabled WRITE setCutawayEnabled NOTIFY cutawayEnabledChanged)
    Q_PROPERTY(int hideWhenEmpty READ hideWhenEmpty WRITE setHideWhenEmpty NOTIFY hideWhenEmptyChanged)

    Q_PROPERTY(QVariantList voices READ voices NOTIFY voicesChanged)
    Q_PROPERTY(QVariantList allStaffTypes READ allStaffTypes NOTIFY allStaffTypesChanged)

    Q_PROPERTY(bool isMainScore READ isMainScore NOTIFY isMainScoreChanged)

    muse::Inject<context::IGlobalContext> context = { this };

public:
    explicit StaffSettingsModel(QObject* parent = nullptr);

    int staffType() const;
    bool isSmallStaff() const;
    bool cutawayEnabled() const;
    int hideWhenEmpty() const;

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
    void setHideWhenEmpty(int value);

signals:
    void staffTypeChanged();
    void voicesChanged();
    void voiceVisibilityChanged(int voiceIndex, bool visible);
    void isSmallStaffChanged();
    void cutawayEnabledChanged();
    void hideWhenEmptyChanged();
    void allStaffTypesChanged();

    void isMainScoreChanged(bool isMainScore);

private:
    notation::INotationPtr currentNotation() const;
    notation::INotationPtr currentMasterNotation() const;
    notation::INotationPartsPtr notationParts() const;
    notation::INotationPartsPtr masterNotationParts() const;

    muse::ID m_staffId;
    QList<bool> m_voicesVisibility;
    notation::StaffConfig m_config;
};
}

#endif // MU_INSTRUMENTSSCENE_STAFFSETTINGSMODEL_H
