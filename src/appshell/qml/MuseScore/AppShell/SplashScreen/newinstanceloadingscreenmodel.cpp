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

#include "newinstanceloadingscreenmodel.h"

#include "translation.h"

using namespace mu::appshell;

NewInstanceLoadingScreenModel::NewInstanceLoadingScreenModel(QObject* parent)
    : QObject(parent), muse::Contextable(nullptr)
{
}

NewInstanceLoadingScreenModel::NewInstanceLoadingScreenModel(bool forNewScore, const QString& openingFileName,
                                                             const muse::modularity::ContextPtr& ctx, QObject* parent)
    : QObject(parent), muse::Contextable(ctx)
{
    if (forNewScore) {
        m_message = muse::qtrc("appshell", "Loading new score…");
        m_width = 288;
        m_height = 80;
    } else if (!openingFileName.isEmpty()) {
        m_message = muse::qtrc("appshell", "Loading “%1”…").arg(openingFileName);
        m_width = 360;
        m_height = 80;
    } else {
        m_message = muse::qtrc("appshell", "Loading score…");
        m_width = 288;
        m_height = 80;
    }
}

QString NewInstanceLoadingScreenModel::message() const
{
    return m_message;
}

int NewInstanceLoadingScreenModel::width() const
{
    return m_width;
}

int NewInstanceLoadingScreenModel::height() const
{
    return m_height;
}

QString NewInstanceLoadingScreenModel::fontFamily() const
{
    if (uiConfiguration()) {
        return QString::fromStdString(uiConfiguration()->fontFamily());
    }
    return QString();
}

int NewInstanceLoadingScreenModel::fontSize() const
{
    if (uiConfiguration()) {
        return uiConfiguration()->fontSize(muse::ui::FontSizeType::BODY_LARGE);
    }
    return 12;
}

QString NewInstanceLoadingScreenModel::backgroundColor() const
{
    if (uiConfiguration()) {
        return uiConfiguration()->currentTheme().values.value(muse::ui::BACKGROUND_PRIMARY_COLOR).toString();
    }
    return "#FFFFFF";
}

QString NewInstanceLoadingScreenModel::messageColor() const
{
    if (uiConfiguration()) {
        return uiConfiguration()->currentTheme().values.value(muse::ui::FONT_PRIMARY_COLOR).toString();
    }
    return "#000000";
}
