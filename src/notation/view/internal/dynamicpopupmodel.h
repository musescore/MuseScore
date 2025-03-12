/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include <QList>
#include <QObject>
#include <QVariantList>

#include "view/abstractelementpopupmodel.h"

namespace mu::notation {
class DynamicPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(QVariantList pages READ pages NOTIFY pagesChanged)

public:
    explicit DynamicPopupModel(QObject* parent = nullptr);

    enum ItemType {
        Dynamic,
        Crescendo,
        Decrescendo
    };
    Q_ENUM(ItemType)

    struct PageItem {
        engraving::DynamicType dynType;
        double width;
        double offset;
        ItemType itemType;
    };

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void addOrChangeDynamic(int page, int index);
    Q_INVOKABLE void addHairpinToDynamic(ItemType itemType);

    Q_INVOKABLE void showPreview(int page, int index);
    Q_INVOKABLE void hidePreview();

    QString fontFamily() const;
    QVariantList pages() const;

signals:
    void pagesChanged();

private:
    // Represents different pages of the popup, each containing dynamic/hairpin symbols as strings, width, offset and ItemType
    QVariantList m_pages;

    QString xmlTextToQString(const std::string& text, engraving::IEngravingFontPtr engravingFont) const;
};
} // namespace mu::notation
