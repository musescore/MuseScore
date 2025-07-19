/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "musescorecomauthorizationmodel.h"

using namespace muse::cloud;

MuseScoreComAuthorizationModel::MuseScoreComAuthorizationModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void MuseScoreComAuthorizationModel::load()
{
    emit userAuthorizedChanged();

    museScoreComService()->authorization()->userAuthorized().ch.onReceive(this, [this](bool) {
        emit userAuthorizedChanged();
    });
}

bool MuseScoreComAuthorizationModel::userAuthorized() const
{
    return museScoreComService()->authorization()->userAuthorized().val;
}

void MuseScoreComAuthorizationModel::createAccount()
{
    museScoreComService()->authorization()->signUp();
}

void MuseScoreComAuthorizationModel::signIn()
{
    museScoreComService()->authorization()->signIn();
}

void MuseScoreComAuthorizationModel::signOut()
{
    museScoreComService()->authorization()->signOut();
}
