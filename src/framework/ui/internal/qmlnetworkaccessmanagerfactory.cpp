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

#include "qmlnetworkaccessmanagerfactory.h"

using namespace muse::ui;

QNetworkReply* QmlNetworkAccessManager::createRequest(Operation operation, const QNetworkRequest& originalRequest, QIODevice* outgoingData)
{
    QNetworkRequest request(originalRequest);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    muse::network::RequestHeaders _headers = networkConfiguration()->defaultHeaders();
    for (auto it = _headers.knownHeaders.cbegin(); it != _headers.knownHeaders.cend(); ++it) {
        request.setHeader(it.key(), it.value());
    }

    for (auto it = _headers.rawHeaders.cbegin(); it != _headers.rawHeaders.cend(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }

    return QNetworkAccessManager::createRequest(operation, request, outgoingData);
}

QNetworkAccessManager* QmlNetworkAccessManagerFactory::create(QObject* parent)
{
    return new QmlNetworkAccessManager(parent);
}
