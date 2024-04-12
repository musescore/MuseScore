/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "notationviewstate.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "notation.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::draw;

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

NotationViewState::NotationViewState(Notation* notation)
{
    notation->openChanged().onNotify(this, [this, notation]() {
        if (!notation->isOpen()) {
            m_isMatrixInited = false;
            setMatrix(Transform(), nullptr);
        }
    });
}

Ret NotationViewState::read(const engraving::MscReader& reader, const muse::io::path_t& pathPrefix)
{
    ByteArray json = reader.readViewSettingsJsonFile(pathPrefix);
    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();
    QJsonObject notationObj = rootObj.value("notation").toObject();

    m_viewMode = viewModeFromString(notationObj.value("viewMode").toString());

    return make_ret(Ret::Code::Ok);
}

Ret NotationViewState::write(engraving::MscWriter& writer, const muse::io::path_t& pathPrefix)
{
    QJsonObject notationObj;
    notationObj["viewMode"] = viewModeToString(m_viewMode);

    QJsonObject rootObj;
    rootObj["notation"] = notationObj;

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeViewSettingsJsonFile(ByteArray::fromQByteArrayNoCopy(json), pathPrefix);

    return make_ret(Ret::Code::Ok);
}

bool NotationViewState::isMatrixInited() const
{
    return m_isMatrixInited;
}

void NotationViewState::setMatrixInited(bool inited)
{
    m_isMatrixInited = inited;
}

Transform NotationViewState::matrix() const
{
    return m_matrix;
}

muse::async::Channel<Transform, NotationPaintView*> NotationViewState::matrixChanged() const
{
    return m_matrixChanged;
}

void NotationViewState::setMatrix(const Transform& matrix, NotationPaintView* sender)
{
    int newZoomPercentage = configuration()->zoomPercentageFromScaling(matrix.m11());
    if (m_matrix == matrix && m_zoomPercentage.val == newZoomPercentage) {
        return;
    }

    m_matrix = matrix;
    m_matrixChanged.send(matrix, sender);
    m_zoomPercentage.set(newZoomPercentage);
}

ValCh<int> NotationViewState::zoomPercentage() const
{
    return m_zoomPercentage;
}

ValCh<ZoomType> NotationViewState::zoomType() const
{
    return m_zoomType;
}

void NotationViewState::setZoomType(ZoomType type)
{
    if (m_zoomType.val != type) {
        m_zoomType.set(type);
    }
}

ViewMode NotationViewState::viewMode() const
{
    return m_viewMode;
}

void NotationViewState::setViewMode(const ViewMode& mode)
{
    if (m_viewMode == mode) {
        return;
    }

    m_viewMode = mode;
    m_stateChanged.notify();
}

void NotationViewState::makeDefault()
{
    m_viewMode = ViewMode::PAGE;
}

muse::async::Notification NotationViewState::stateChanged() const
{
    return m_stateChanged;
}
