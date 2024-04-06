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
#ifndef MUSE_NETWORK_NETWORKTYPES_H
#define MUSE_NETWORK_NETWORKTYPES_H

#include <QVariantMap>
#include <QNetworkRequest>

class QHttpMultiPart;
class QIODevice;

namespace muse::network {
struct RequestHeaders
{
    QMap<QNetworkRequest::KnownHeaders, QVariant> knownHeaders;
    QMap<QByteArray, QByteArray> rawHeaders;
};

using IncomingDevice = QIODevice;

struct OutgoingDevice
{
    OutgoingDevice(QIODevice* device)
        : m_device(device), m_multiPart(nullptr) {}
    OutgoingDevice(QHttpMultiPart* multiPart)
        : m_device(nullptr), m_multiPart(multiPart) {}

    QIODevice* device() const
    {
        return m_device;
    }

    QHttpMultiPart* multiPart() const
    {
        return m_multiPart;
    }

private:
    QIODevice* m_device = nullptr;
    QHttpMultiPart* m_multiPart = nullptr;
};
}

#endif // MUSE_NETWORK_NETWORKTYPES_H
