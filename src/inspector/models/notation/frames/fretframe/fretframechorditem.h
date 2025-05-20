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

#include "uicomponents/view/selectableitemlistmodel.h"

namespace mu::inspector {
class FretFrameChordItem : public muse::uicomponents::SelectableItemListModel::Item
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool isVisible READ isVisible WRITE setIsVisible NOTIFY isVisibleChanged)

public:
    explicit FretFrameChordItem(QObject* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    bool isVisible() const;
    void setIsVisible(bool visible);

signals:
    void titleChanged();
    void isVisibleChanged();

private:
    QString m_title;
    bool m_isVisible = false;
};
}
