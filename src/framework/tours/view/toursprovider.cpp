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
#include "toursprovider.h"

#include <QApplication>

#include "log.h"

static const char* SHOW_TOOLTIP_PROPERTY_NAME("toolTipShowLocked");

using namespace muse;
using namespace muse::tours;

ToursProvider::ToursProvider(const modularity::ContextPtr& iocCtx)
    : QObject(), Injectable(iocCtx)
{
    connect(&m_openTimer, &QTimer::timeout, this, &ToursProvider::doShow);

    connect(qApp, &QApplication::applicationStateChanged, this, &ToursProvider::onApplicationStateChanged);
}

void ToursProvider::showTour(const Tour& tour)
{
    m_tour = tour;
    m_currentStep = 0;
    m_totalSteps = m_tour.steps.size();

    showNext();
}

void ToursProvider::showNext()
{
    if (m_currentStep >= m_totalSteps) {
        clear();
        return;
    }

    m_openTimer.start();
}

void ToursProvider::onTourStepClosed(QQuickItem* parentItem)
{
    //! NOTE: Restore showing tooltip for tour's control
    setBlockShowingTooltipForItem(parentItem, false);
}

void ToursProvider::doShow()
{
    m_openTimer.stop();

    if (qApp->applicationState() != Qt::ApplicationActive) {
        m_needShowTourAfterApplicationActivation = true;
        return;
    }

    const TourStep& step = m_tour.steps[m_currentStep];
    size_t index = m_currentStep + 1;

    m_currentStep++;

    const ui::INavigationControl* parentControl = findControl(step.controlUri);
    IF_ASSERT_FAILED(parentControl) {
        return;
    }

    QQuickItem* parentControlItem = parentControl->visualItem();
    IF_ASSERT_FAILED(parentControlItem) {
        return;
    }

    parentControl->triggered().onNotify(this, [this]() {
        emit closeCurrentTourStep();
        showNext();
    });

    //! NOTE: Avoid showing tooltip for control when tour for that control is shown
    setBlockShowingTooltipForItem(parentControlItem, true);

    emit openTourStep(parentControlItem, step.title, step.description, step.videoExplanationUrl, index, m_totalSteps);
}

const ui::INavigationControl* ToursProvider::findControl(const Uri& controlUri)
{
    String controlPath = String::fromStdString(controlUri.path());

    StringList pathItems = controlPath.split('/');
    IF_ASSERT_FAILED(pathItems.size() == 3) {
        LOGE() << "Invalid control uri: " << controlUri;
        return nullptr;
    }

    std::string section = pathItems[0].toStdString();
    std::string panel = pathItems[1].toStdString();
    std::string controlName = pathItems[2].toStdString();

    return navigationController()->findControl(section, panel, controlName);
}

void ToursProvider::clear()
{
    m_openTimer.stop();

    m_tour = Tour();
    m_currentStep = 0;
    m_totalSteps = 0;
}

void ToursProvider::setBlockShowingTooltipForItem(QQuickItem* item, bool block)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    item->setProperty(SHOW_TOOLTIP_PROPERTY_NAME, QVariant::fromValue(block));
}

void ToursProvider::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (!m_needShowTourAfterApplicationActivation) {
        return;
    }

    m_needShowTourAfterApplicationActivation = false;

    if (state == Qt::ApplicationActive) {
        doShow();
    }
}

bool ToursProvider::canControlTourPopupClosing() const
{
#ifdef Q_OS_LINUX
    //! NOTE: There is a problem on Linux for popups that when you turn off automatic closing,
    //! if the user switches to another program, the popup will still be shown on top of the other program.
    //! Let's turn on automatic closing of the popup for Linux.
    return false;
#else
    return true;
#endif
}
