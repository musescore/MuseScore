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

#include <QObject>

#include "view/abstractelementpopupmodel.h"

namespace mu::notation {
class ShadowNotePopupContent
{
    Q_GADGET
public:
    enum ContentType
    {
        NONE = -1,
        PERCUSSION_CONTENT
    };
    Q_ENUM(ContentType)
};

class ShadowNotePopupModel : public AbstractElementPopupModel
{
    Q_OBJECT
    Q_PROPERTY(ShadowNotePopupContent::ContentType currentPopupType
               READ currentPopupType NOTIFY currentPopupTypeChanged)

public:
    explicit ShadowNotePopupModel(QObject* parent = nullptr);

    static bool canOpen(const EngravingItem* shadowNote);

    Q_INVOKABLE void init() override;

    ShadowNotePopupContent::ContentType currentPopupType() const;

signals:
    void currentPopupTypeChanged();

protected:
    void updateItemRect() override;
};
} // namespace mu::notation
