//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
    void setIcon(framework::IconCode::Code icon);
    void setChecked(bool checked);

signals:
    void iconChanged(int icon);
    void checkedChanged(bool checked);

private:
    framework::IconCode::Code m_icon = framework::IconCode::Code::NONE;
    bool m_checked = false;
};
}

#endif // MU_NOTATION_ACTIONNOTEINPUTBARITEM_H
