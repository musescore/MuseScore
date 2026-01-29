/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "interactiveprovidermodel.h"

using namespace muse::interactive;

InteractiveProviderModel::InteractiveProviderModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void InteractiveProviderModel::classBegin()
{
    provider()->openRequested().onReceive(this, [this](QmlLaunchData* data) {
        emit openRequested(data);
    });

    provider()->closeRequested().onReceive(this, [this](const QVariant& objectId) {
        emit closeRequested(objectId);
    });

    provider()->raiseRequested().onReceive(this, [this](const QVariant& objectId) {
        emit raiseRequested(objectId);
    });
}

QWindow* InteractiveProviderModel::topWindow() const
{
    return provider()->topWindow();
}

QString InteractiveProviderModel::objectId(const QVariant& val) const
{
    return provider()->objectId(val);
}

void InteractiveProviderModel::onOpen(const QVariant& type, const QVariant& objectId, QObject* window)
{
    provider()->onOpen(type, objectId, window);
}

void InteractiveProviderModel::onClose(const QString& objectId, const QVariant& jsrv)
{
    provider()->onClose(objectId, jsrv);
}
