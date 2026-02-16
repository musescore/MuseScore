/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "loadingscreenmodel.h"

#include "translation.h"

using namespace mu::appshell;

LoadingScreenModel::LoadingScreenModel(QObject* parent)
    : QObject(parent), muse::Contextable(nullptr)
{
}

LoadingScreenModel::LoadingScreenModel(const muse::modularity::ContextPtr& ctx, QObject* parent)
    : QObject(parent), muse::Contextable(ctx)
{
}

QString LoadingScreenModel::message() const
{
    return muse::qtrc("appshell", "Loading…");
}

QString LoadingScreenModel::website() const
{
    return QString::fromStdString(globalConfiguration()->museScoreUrl());
}

QString LoadingScreenModel::versionString() const
{
    if (application()) {
        return muse::qtrc("appshell", "Version %1").arg(application()->fullVersion().toString());
    }
    return QString();
}

QString LoadingScreenModel::fontFamily() const
{
    if (uiConfiguration()) {
        return QString::fromStdString(uiConfiguration()->fontFamily());
    }
    return QString();
}

int LoadingScreenModel::fontSize() const
{
    if (uiConfiguration()) {
        return uiConfiguration()->fontSize();
    }
    return 12;
}

bool LoadingScreenModel::isRtl() const
{
    if (languagesService()) {
        return languagesService()->currentLanguage().direction == Qt::RightToLeft;
    }
    return false;
}
