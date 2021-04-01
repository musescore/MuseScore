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

#include "commonaudioapiconfigurationmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::appshell;

CommonAudioApiConfigurationModel::CommonAudioApiConfigurationModel(QObject* parent)
    : QObject(parent)
{
}

int CommonAudioApiConfigurationModel::currentDeviceIndex() const
{
    return m_currentDeviceIndex;
}

int CommonAudioApiConfigurationModel::currentSampleRateIndex() const
{
    return m_currentSampleRateIndex;
}

void CommonAudioApiConfigurationModel::setCurrentDeviceIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentDeviceIndex()) {
        return;
    }

    m_currentDeviceIndex = index;
    emit currentDeviceIndexChanged(index);
}

void CommonAudioApiConfigurationModel::setCurrentSampleRateIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentDeviceIndex()) {
        return;
    }

    m_currentSampleRateIndex = index;
    emit currentSampleRateIndexChanged(index);
}

QStringList CommonAudioApiConfigurationModel::deviceList() const
{
    QStringList devices {
        "Built-in Output",
        "Test device 1",
        "Test device 2"
    };

    return devices;
}

QStringList CommonAudioApiConfigurationModel::sampleRateHzList() const
{
    QList<int> sampleRateList {
        192000,
        96000,
        88200,
        48000,
        44100,
        32000,
        22050
    };

    QStringList result;
    for (int sampleRate : sampleRateList) {
        result << QString::number(sampleRate) + " " + qtrc("global", "Hz");
    }

    return result;
}
