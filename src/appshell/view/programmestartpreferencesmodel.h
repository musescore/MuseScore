//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H
#define MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H

#include <QObject>

namespace mu::appshell {
class ProgrammeStartPreferencesModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList startModes READ startModes NOTIFY startModesChanged)
    Q_PROPERTY(QVariantList panels READ panels NOTIFY panelsChanged)

public:
    explicit ProgrammeStartPreferencesModel(QObject* parent = nullptr);

    QVariantList startModes() const;
    QVariantList panels() const;

    Q_INVOKABLE void setCurrentStartMode(int modeIndex, const QString& scorePath);
    Q_INVOKABLE void setPanelVisible(int panelIndex, bool visible);

signals:
    void startModesChanged();
    void panelsChanged();
};
}

#endif // MU_APPSHELL_GENERALPREFERENCESMODEL_H
