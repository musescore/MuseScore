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
#ifndef MU_APPSHELL_UPDATEPREFERENCESMODEL_H
#define MU_APPSHELL_UPDATEPREFERENCESMODEL_H

#include <QObject>

namespace mu::appshell {
class UpdatePreferencesModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        bool needCheckForNewMuseScoreVersion READ needCheckForNewMuseScoreVersion WRITE setNeedCheckForNewMuseScoreVersion NOTIFY needCheckForNewMuseScoreVersionChanged)
    Q_PROPERTY(
        bool needCheckForNewExtensionsVersion READ needCheckForNewExtensionsVersion WRITE setNeedCheckForNewExtensionsVersion NOTIFY needCheckForNewMuseScoreVersionChanged)

public:
    explicit UpdatePreferencesModel(QObject* parent = nullptr);

    bool needCheckForNewMuseScoreVersion() const;
    bool needCheckForNewExtensionsVersion() const;

    Q_INVOKABLE void load();

public slots:
    void setNeedCheckForNewMuseScoreVersion(bool value);
    void setNeedCheckForNewExtensionsVersion(bool value);

signals:
    void needCheckForNewMuseScoreVersionChanged(bool value);
    void needCheckForNewExtensionsVersionChanged(bool value);
};
}

#endif // MU_APPSHELL_UPDATEPREFERENCESMODEL_H
