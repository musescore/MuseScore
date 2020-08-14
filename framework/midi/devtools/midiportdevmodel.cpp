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
        list << item;
    }

    return list;
}
