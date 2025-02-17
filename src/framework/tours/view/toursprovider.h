/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include <QQuickItem>
#include <QTimer>

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

#include "../itoursprovider.h"

namespace muse::tours {
class ToursProvider : public QObject, public IToursProvider, public Injectable
{
    Q_OBJECT

    Inject<ui::INavigationController> navigationController = { this };

public:
    explicit ToursProvider(const modularity::ContextPtr& iocCtx);

    void showTour(const Tour& tour) override;

    Q_INVOKABLE void showNext();

private slots:
    void doShow();

signals:
    void openTourStep(const QQuickItem* parentItem, const QString& title, const QString& description, const QString& videoExplanationUrl,
                      size_t index, size_t total);

private:
    QQuickItem* findControl(const Uri& controlUri);

    void onApplicationStateChanged(Qt::ApplicationState state);

    QTimer m_openTimer;

    Tour m_tour;
    size_t m_currentStep = 0;
    size_t m_totalSteps = 0;

    bool m_needShowTourAfterApplicationActivation = false;
};
}
