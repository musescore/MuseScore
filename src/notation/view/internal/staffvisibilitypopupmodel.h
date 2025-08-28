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

#pragma once

#include <QAbstractItemModel>
#include <QQmlParserStatus>
#include <memory>

#include "emptystavesvisibilitymodel.h"
#include "view/abstractelementpopupmodel.h"

namespace mu::engraving {
class Measure;
class System;
}

namespace mu::notation {
class EmptyStavesVisibilityModel;
class StaffVisibilityPopupModel : public AbstractElementPopupModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(EmptyStavesVisibilityModel * emptyStavesVisibilityModel READ emptyStavesVisibilityModel CONSTANT)
    Q_PROPERTY(size_t systemIndex READ systemIndex NOTIFY systemIndexChanged)

public:
    explicit StaffVisibilityPopupModel(QObject* parent = nullptr);

    Q_INVOKABLE void init() override;

    EmptyStavesVisibilityModel* emptyStavesVisibilityModel() const { return m_emptyStavesVisibilityModel.get(); }
    size_t systemIndex() const { return m_systemIndex; }

signals:
    void systemIndexChanged();

private:
    void classBegin() override;
    void componentComplete() override {}

    std::unique_ptr<EmptyStavesVisibilityModel> m_emptyStavesVisibilityModel = nullptr;
    size_t m_systemIndex = 0;
};
}
