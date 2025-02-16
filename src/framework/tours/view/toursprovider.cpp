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

#include "log.h"

using namespace muse;
using namespace muse::tours;

ToursProvider::ToursProvider(const modularity::ContextPtr& iocCtx)
    : QObject(), Injectable(iocCtx)
{
    QObject::connect(&m_openTimer, &QTimer::timeout, this, &ToursProvider::doShow);
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
    m_openTimer.start();
}

void ToursProvider::doShow()
{
    m_openTimer.stop();

    const TourStep& step = m_tour.steps[m_currentStep];
    size_t index = m_currentStep + 1;

    m_currentStep++;

    QQuickItem* parentItem = findControl(step.controlUri);
    IF_ASSERT_FAILED(parentItem) {
        return;
    }

    emit openTourStep(parentItem, step.title, step.description, step.videoExplanationUrl, index, m_totalSteps);
}

QQuickItem* ToursProvider::findControl(const Uri& controlUri)
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

    const ui::INavigationControl* control = navigationController()->findControl(section, panel, controlName);
    return control ? control->visualItem() : nullptr;
}
