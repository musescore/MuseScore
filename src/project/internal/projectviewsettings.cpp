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

#include "projectviewsettings.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace mu;
using namespace mu::project;
using namespace mu::engraving;
using namespace mu::notation;

static ViewMode viewModeFromString(const QString& str)
{
    if ("page" == str) {
        return ViewMode::PAGE;
    }

    if ("float" == str) {
        return ViewMode::FLOAT;
    }

    if ("continuous_v" == str) {
        return ViewMode::LINE;
    }

    if ("continuous_h" == str) {
        return ViewMode::SYSTEM;
    }

    if ("continuous_h_fixed" == str) {
        return ViewMode::HORIZONTAL_FIXED;
    }

    return ViewMode::PAGE;
}

static QString viewModeToString(ViewMode m)
{
    switch (m) {
    case ViewMode::PAGE: return "page";
    case ViewMode::FLOAT: return "float";
    case ViewMode::LINE: return "continuous_v";
    case ViewMode::SYSTEM: return "continuous_h";
    case ViewMode::HORIZONTAL_FIXED: return "continuous_h_fixed";
    }
    return "";
}

Ret ProjectViewSettings::read(const MscReader& reader)
{
    QByteArray json = reader.readViewSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json).object();
    QJsonObject notationObj = rootObj.value("notation").toObject();

    m_viewMode = viewModeFromString(notationObj.value("viewMode").toString());

    return make_ret(Ret::Code::Ok);
}

Ret ProjectViewSettings::write(MscWriter& writer)
{
    QJsonObject notationObj;
    notationObj["viewMode"] = viewModeToString(m_viewMode);

    QJsonObject rootObj;
    rootObj["notation"] = notationObj;

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeViewSettingsJsonFile(json);

    setNeedSave(false);

    return make_ret(Ret::Code::Ok);
}

void ProjectViewSettings::makeDefault()
{
    m_viewMode = ViewMode::PAGE;
}

void ProjectViewSettings::setNeedSave(bool needSave)
{
    if (m_needSave == needSave) {
        return;
    }

    m_needSave = needSave;
    m_needSaveNotification.notify();
}

ViewMode ProjectViewSettings::notationViewMode() const
{
    return m_viewMode;
}

void ProjectViewSettings::setNotationViewMode(const ViewMode& mode)
{
    if (m_viewMode == mode) {
        return;
    }

    m_viewMode = mode;
    setNeedSave(true);
}

mu::ValNt<bool> ProjectViewSettings::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = m_needSave;
    needSave.notification = m_needSaveNotification;

    return needSave;
}
