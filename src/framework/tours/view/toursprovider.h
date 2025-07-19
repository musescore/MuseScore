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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

#include "../itoursprovider.h"

namespace muse::tours {
class ToursProvider : public QObject, public IToursProvider, public async::Asyncable, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(bool canControlTourPopupClosing READ canControlTourPopupClosing CONSTANT)

    Inject<ui::INavigationController> navigationController = { this };

public:
    explicit ToursProvider(const modularity::ContextPtr& iocCtx);

    void showTour(const Tour& tour) override;

    Q_INVOKABLE void showNext();

    Q_INVOKABLE void onTourStepClosed(QQuickItem* parentItem);

    bool canControlTourPopupClosing() const;

private slots:
    void doShow();

signals:
    void openTourStep(const QQuickItem* parentItem, const QString& title, const QString& description, const QString& previewImageOrGifUrl,
                      const QString& videoExplanationUrl, size_t index, size_t total);
    void closeCurrentTourStep();

private:
    const ui::INavigationControl* findControl(const Uri& controlUri);

    void clear();

    void setBlockShowingTooltipForItem(QQuickItem* item, bool block);

    void onApplicationStateChanged(Qt::ApplicationState state);

    QTimer m_openTimer;

    Tour m_tour;
    size_t m_currentStep = 0;
    size_t m_totalSteps = 0;

    bool m_needShowTourAfterApplicationActivation = false;
};
}
