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
#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "internal/iinteractiveprovider.h"

namespace muse::interactive {
struct QmlLaunchDataForeign {
    Q_GADGET
    QML_FOREIGN(muse::interactive::QmlLaunchData)
    QML_ANONYMOUS;
    QML_UNCREATABLE("Must be created in C++ only")
};

class InteractiveProviderModel : public QObject, public QQmlParserStatus, public Injectable, public async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    QML_ELEMENT

    Inject<IInteractiveProvider> provider = { this };

public:
    explicit InteractiveProviderModel(QObject* parent = nullptr);

    Q_INVOKABLE QWindow* topWindow() const;

    Q_INVOKABLE QString objectId(const QVariant& val) const;

    Q_INVOKABLE void onOpen(const QVariant& type, const QVariant& objectId, QObject* window = nullptr);
    Q_INVOKABLE void onClose(const QString& objectId, const QVariant& rv);

signals:
    void openRequested(muse::interactive::QmlLaunchData* data);
    void closeRequested(const QVariant& data);
    void raiseRequested(const QVariant& data);

private:
    void classBegin() override;
    void componentComplete() override {}
};
}
