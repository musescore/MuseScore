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

#include "modularity/ioc.h"
#include "ui_text_position_select.h"
#include "engraving/types/types.h"

namespace mu::notation {
class TextPositionSelect : public QWidget, public Ui::PositionSelect, public muse::Injectable
{
    Q_OBJECT

public:
    TextPositionSelect(QWidget* parent);
    mu::engraving::AlignH position() const;
    void setPosition(mu::engraving::AlignH);

signals:
    void positionChanged(mu::engraving::AlignH);

private:
    QButtonGroup* positionButtons;

    void blockPosition(bool val);

private slots:
    void _positionChanged();
};
}
