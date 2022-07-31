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
#include <QJsonArray>

#include "types/bytearray.h"

using namespace mu;
using namespace mu::project;
using namespace mu::engraving;
using namespace mu::notation;

#define DEFAULT_VIEW_MODE ViewMode::PAGE

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

    return DEFAULT_VIEW_MODE;
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
    ByteArray json = reader.readViewSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();
    QJsonObject notationObj = rootObj.value("notation").toObject();

    m_masterViewMode = viewModeFromString(notationObj.value("viewMode").toString());

    if (notationObj.contains("excerpts")) {
        QJsonArray excerptsArray = notationObj.value("excerpts").toArray();

        m_excerptsViewModes.clear();
        m_excerptsViewModes.reserve(excerptsArray.size());

        for (const QJsonValue& excerptVal : excerptsArray) {
            QJsonObject excerptObj = excerptVal.toObject();
            m_excerptsViewModes.push_back(viewModeFromString(excerptObj.value("viewMode").toString()));
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret ProjectViewSettings::write(MscWriter& writer)
{
    QJsonArray excerptsArray;

    for (const notation::ViewMode& mode : m_excerptsViewModes) {
        // Use an object to allow to store more values in the future if needed
        QJsonObject excerptObj;
        excerptObj["viewMode"] = viewModeToString(mode);

        excerptsArray.append(excerptObj);
    }

    QJsonObject notationObj;
    notationObj["viewMode"] = viewModeToString(m_masterViewMode);
    notationObj["excerpts"] = excerptsArray;

    QJsonObject rootObj;
    rootObj["notation"] = notationObj;

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeViewSettingsJsonFile(ByteArray::fromQByteArrayNoCopy(json));

    setNeedSave(false);

    return make_ret(Ret::Code::Ok);
}

void ProjectViewSettings::makeDefault()
{
    m_masterViewMode = DEFAULT_VIEW_MODE;
    m_excerptsViewModes.clear();
}

void ProjectViewSettings::setNeedSave(bool needSave)
{
    if (m_needSave == needSave) {
        return;
    }

    m_needSave = needSave;
    m_needSaveNotification.notify();
}

ViewMode ProjectViewSettings::masterNotationViewMode() const
{
    return m_masterViewMode;
}

void ProjectViewSettings::setMasterNotationViewMode(const ViewMode& mode)
{
    if (m_masterViewMode == mode) {
        return;
    }

    m_masterViewMode = mode;
    setNeedSave(true);
}

ViewMode ProjectViewSettings::excerptViewMode(size_t excerptIndex) const
{
    if (excerptIndex >= m_excerptsViewModes.size()) {
        return DEFAULT_VIEW_MODE;
    }

    return m_excerptsViewModes[excerptIndex];
}

void ProjectViewSettings::setExcerptViewMode(size_t excerptIndex, const ViewMode& mode)
{
    // Avoid big resize if the index is too big by mistake
    assert(excerptIndex < 10000);

    if (excerptIndex >= m_excerptsViewModes.size()) {
        m_excerptsViewModes.resize(excerptIndex + 1, DEFAULT_VIEW_MODE);
    }

    if (m_excerptsViewModes[excerptIndex] == mode) {
        return;
    }

    m_excerptsViewModes[excerptIndex] = mode;
    setNeedSave(true);
}

void ProjectViewSettings::removeExcerpt(size_t excerptIndex)
{
    if (excerptIndex >= m_excerptsViewModes.size()) {
        return;
    }

    // Since the excerpt view mode use the index of the excerpt in the master score,
    // if a excerpt is removed, we need to shift the index of the following excerpts.
    m_excerptsViewModes.erase(m_excerptsViewModes.begin() + excerptIndex);
}

mu::ValNt<bool> ProjectViewSettings::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = m_needSave;
    needSave.notification = m_needSaveNotification;

    return needSave;
}
