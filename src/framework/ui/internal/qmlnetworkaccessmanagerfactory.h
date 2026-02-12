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

#include <QQmlNetworkAccessManagerFactory>
#include <QNetworkAccessManager>

#include "global/modularity/ioc.h"
#include "network/inetworkconfiguration.h"

namespace muse::ui {
class QmlNetworkAccessManager : public QNetworkAccessManager
{
    muse::GlobalInject<muse::network::INetworkConfiguration> networkConfiguration;

public:
    explicit QmlNetworkAccessManager(QObject* parent = nullptr)
        : QNetworkAccessManager(parent) {}

protected:
    QNetworkReply* createRequest(Operation operation, const QNetworkRequest& originalRequest, QIODevice* outgoingData) override;
};

class QmlNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QNetworkAccessManager* create(QObject* parent) override;
};
}
