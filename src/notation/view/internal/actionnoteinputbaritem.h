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
#ifndef MU_NOTATION_ACTIONNOTEINPUTBARITEM_H
#define MU_NOTATION_ACTIONNOTEINPUTBARITEM_H

#include "abstractnoteinputbaritem.h"
#include "ui/view/iconcodes.h"

namespace mu::notation {
class ActionNoteInputBarItem : public AbstractNoteInputBarItem
{
    Q_OBJECT

    Q_PROPERTY(int icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)

public:
    explicit ActionNoteInputBarItem(const ItemType& type, QObject* parent = nullptr);

    int icon() const;
    bool checked() const;

public slots:
    void setIcon(ui::IconCode::Code icon);
    void setChecked(bool checked);

signals:
    void iconChanged(int icon);
    void checkedChanged(bool checked);

private:
    ui::IconCode::Code m_icon = ui::IconCode::Code::NONE;
    bool m_checked = false;
};
}

#endif // MU_NOTATION_ACTIONNOTEINPUTBARITEM_H
