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
#include "toursprovidermodel.h"

using namespace muse::tours;

ToursProviderModel::ToursProviderModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void ToursProviderModel::classBegin()
{
    // TODO: avoid direct usage of ToursProvider, and use IToursProvider only
    ToursProvider* providerPtr = toursProvider();
    connect(providerPtr, &ToursProvider::openTourStep, this, &ToursProviderModel::openTourStep);
    connect(providerPtr, &ToursProvider::closeCurrentTourStep, this, &ToursProviderModel::closeCurrentTourStep);
}

void ToursProviderModel::showNext()
{
    toursProvider()->showNext();
}

void ToursProviderModel::onTourStepClosed(QQuickItem* parentItem)
{
    toursProvider()->onTourStepClosed(parentItem);
}

bool ToursProviderModel::canControlTourPopupClosing() const
{
    return toursProvider()->canControlTourPopupClosing();
}

muse::tours::ToursProvider* ToursProviderModel::toursProvider() const
{
    return dynamic_cast<tours::ToursProvider*>(provider().get());
}
