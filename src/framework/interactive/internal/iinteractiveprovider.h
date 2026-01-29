/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QObject>
#include <QVariant>

#include "global/async/channel.h"

#include "global/modularity/imoduleinterface.h"

namespace muse::interactive {
class QmlLaunchData : public QObject
{
    Q_OBJECT
public:
    explicit QmlLaunchData(QObject* parent = nullptr);

    Q_INVOKABLE QVariant value(const QString& key) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& val);
    Q_INVOKABLE QVariant data() const;

private:
    QVariantMap m_data;
};

/// Internal interface for communication between Interactive and InteractiveProviderModel
class IInteractiveProvider : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IInteractiveProvider)

public:
    virtual ~IInteractiveProvider() = default;

    virtual QWindow* topWindow() const = 0;

    virtual QString objectId(const QVariant& val) const = 0;

    virtual void onOpen(const QVariant& type, const QVariant& objectId, QObject* window = nullptr) = 0;
    virtual void onClose(const QString& objectId, const QVariant& rv) = 0;

    virtual async::Channel<QmlLaunchData*> openRequested() const = 0;
    virtual async::Channel<QVariant /*objectId*/> closeRequested() const = 0;
    virtual async::Channel<QVariant /*objectId*/> raiseRequested() const = 0;
};
}
