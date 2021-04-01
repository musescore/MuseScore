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
#ifndef MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H
#define MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H

#include <QObject>

namespace mu::appshell {
class CommonAudioApiConfigurationModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(int currentSampleRateIndex READ currentSampleRateIndex WRITE setCurrentSampleRateIndex NOTIFY currentSampleRateIndexChanged)

public:
    explicit CommonAudioApiConfigurationModel(QObject* parent = nullptr);

    int currentDeviceIndex() const;
    int currentSampleRateIndex() const;

    Q_INVOKABLE QStringList deviceList() const;
    Q_INVOKABLE QStringList sampleRateHzList() const;

public slots:
    void setCurrentDeviceIndex(int index);
    void setCurrentSampleRateIndex(int index);

signals:
    void currentDeviceIndexChanged(int index);
    void currentSampleRateIndexChanged(int index);

private:
    int m_currentDeviceIndex = 0;
    int m_currentSampleRateIndex = 0;
};
}

#endif // MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H
