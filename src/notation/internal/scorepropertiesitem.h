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

#ifndef MU_NOTATION_SCOREPROPERTIESITEM_H
#define MU_NOTATION_SCOREPROPERTIESITEM_H

#include <QLineEdit>
#include <QLayoutItem>

namespace mu::notation {

    //---------------------------------------------------------
    //  Class for Score Property Items
    //  Used in the Score Properties UI to get name value pairs 
    //---------------------------------------------------------
    class ScorePropertiesItem
    {
    public:
        ScorePropertiesItem(QWidget* label = nullptr, QWidget* value = nullptr);
        QString ScorePropertiesItem::Label();
        QString ScorePropertiesItem::Value();
        void ScorePropertiesItem::SetFocus();

    private:
        QWidget* label;
        QWidget* value;
    };
}

#endif // MU_NOTATION_SCOREPROPERTIESITEM_H
