//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "midiportdevmodel.h"

#include "log.h"

using namespace mu::midi;

MidiPortDevModel::MidiPortDevModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList MidiPortDevModel::outputDevices() const
{
    QVariantList list;
    std::vector<IMidiOutPort::Device> devs = midiOutPort()->devices();
    for (const IMidiOutPort::Device& d : devs) {
        QVariantMap item;
        item["id"] = QString::fromStdString(d.id);
        item["name"] = QString::fromStdString(d.name);

        bool isConnected = midiOutPort()->connectedDeviceID() == d.id;

        item["action"] = isConnected ? "Disconnect" : "Connect";

        item["error"] = m_connectionErrors.value(QString::fromStdString(d.id));
        list << item;
    }

    return list;
}

void MidiPortDevModel::outputDeviceAction(const QString& deviceID, const QString& action)
{
    LOGI() << "deviceID: " << deviceID << ", action: " << action;
    midiOutPort()->disconnect();

    if (action == "Connect") {
        Ret ret = midiOutPort()->connect(deviceID.toStdString());
        if (!ret) {
            LOGE() << "failed connect, deviceID: " << deviceID << ", err: " << ret.text();
            m_connectionErrors[deviceID] = QString::fromStdString(ret.text());
        }
    }

    emit outputDevicesChanged();
}
