/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_APPSHELL_BRAILLEPREFERENCESMODEL_H
#define MU_APPSHELL_BRAILLEPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "braille/ibrailleconfiguration.h"

namespace mu::appshell {
class BraillePreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(braille::IBrailleConfiguration, brailleConfiguration)

    Q_PROPERTY(bool braillePanelEnabled READ braillePanelEnabled WRITE setBraillePanelEnabled NOTIFY braillePanelEnabledChanged)
    Q_PROPERTY(QString brailleTable READ brailleTable WRITE setBrailleTable NOTIFY brailleTableChanged)
    Q_PROPERTY(QString intervalDirection READ intervalDirection WRITE setIntervalDirection NOTIFY intervalDirectionChanged)

public:
    explicit BraillePreferencesModel(QObject* parent = nullptr);

    bool braillePanelEnabled() const;
    QString brailleTable() const;
    QString intervalDirection() const;

    Q_INVOKABLE QStringList brailleTables() const;
    Q_INVOKABLE QStringList intervalDirections() const;

public slots:
    void setBraillePanelEnabled(bool value);
    void setBrailleTable(QString table);
    void setIntervalDirection(QString direction);

signals:
    void braillePanelEnabledChanged(bool value);
    void brailleTableChanged(QString value);
    void intervalDirectionChanged(QString value);
};
}

#endif // MU_APPSHELL_BRAILLEPREFERENCESMODEL_H
