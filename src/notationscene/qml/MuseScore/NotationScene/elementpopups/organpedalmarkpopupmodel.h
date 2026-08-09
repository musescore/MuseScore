/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include <QObject>

#include "../abstractelementpopupmodel.h"

namespace mu::notation {
class OrganPedalMarkPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList pages READ pages NOTIFY pagesChanged)

    Q_PROPERTY(bool placeAbove READ placeAbove NOTIFY placeAboveChanged)

public:
    explicit OrganPedalMarkPopupModel(QObject* parent = nullptr);

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void changePedalMark(int popupPageIndex, int pageElementIndex);

    QVariantList pages() const;

    bool placeAbove() const;

signals:
    void pagesChanged();
    void placeAboveChanged();

private:
    QVariantList m_pages;

    void updateItemRect() override;
    bool m_placeAbove = true;
};
}