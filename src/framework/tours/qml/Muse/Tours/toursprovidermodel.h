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

#include <QObject>
#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "internal/toursprovider.h"

namespace muse::tours {
class ToursProviderModel : public QObject, public QQmlParserStatus, public muse::Injectable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool canControlTourPopupClosing READ canControlTourPopupClosing CONSTANT)

    QML_ELEMENT

    Inject<IToursProvider> provider = { this };

public:
    explicit ToursProviderModel(QObject* parent = nullptr);

    Q_INVOKABLE void showNext();
    Q_INVOKABLE void onTourStepClosed(QQuickItem* parentItem);

    bool canControlTourPopupClosing() const;

signals:
    void openTourStep(const QQuickItem* parentItem, const QString& title, const QString& description, const QString& previewImageOrGifUrl,
                      const QString& videoExplanationUrl, int index, int total);
    void closeCurrentTourStep();

private:

    void classBegin() override;
    void componentComplete() override {}

    muse::tours::ToursProvider* toursProvider() const;
};
}
