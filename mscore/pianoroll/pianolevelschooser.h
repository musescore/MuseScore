//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __PIANOLEVELSCHOOSER_H__
#define __PIANOLEVELSCHOOSER_H__

#include "ui_pianolevelschooser.h"

#include "libmscore/staff.h"

namespace Ms {
class PianoView;

//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

class PianoLevelsChooser : public QWidget, public Ui::PianoLevelsChooser
{
    Q_OBJECT

    int _levelsIndex;
    Staff* _staff;
    PianoView* _pianoView = nullptr;

public:
    Staff* staff() { return _staff; }
    void setStaff(Staff* staff) { _staff = staff; }
    void setPianoView(PianoView* pianoView);

signals:
    void levelsIndexChanged(int);
    void notesChanged();

public slots:
    void updateSetboxValue();
    void setLevelsIndex(int index);
    void setEventDataPressed();

public:
    explicit PianoLevelsChooser(QWidget* parent = 0);
};
}

#endif // __PIANOLEVELSCHOOSER_H__
