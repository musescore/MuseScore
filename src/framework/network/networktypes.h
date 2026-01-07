/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include <QVariantMap>
#include <QNetworkRequest>

class QHttpMultiPart;
class QIODevice;

namespace muse::network {
struct RequestHeaders
{
    QMap<QNetworkRequest::KnownHeaders, QVariant> knownHeaders;
    QMap<QByteArray, QByteArray> rawHeaders;

    bool isEmpty() const
    {
        return knownHeaders.isEmpty() && rawHeaders.isEmpty();
    }
};

using QIODevicePtr = std::shared_ptr<QIODevice>;
using IncomingDevice = QIODevice;
using IncomingDevicePtr = QIODevicePtr;

struct NoOutgoingDevice {};
using QHttpMultiPartPtr = std::shared_ptr<QHttpMultiPart>;
using OutgoingDeviceVar = std::variant<QIODevicePtr, QHttpMultiPartPtr, NoOutgoingDevice>;
}
