//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __WORKSPACECOMBOBOX_H__
#define __WORKSPACECOMBOBOX_H__

namespace Ms {
class MuseScore;

//---------------------------------------------------------
//   WorkspaceComboBox
//---------------------------------------------------------

class WorkspaceComboBox : public QComboBox
{
    Q_OBJECT

    MuseScore * _mscore;

    bool blockUpdateWorkspaces = false;

    void changeEvent(QEvent* event) override;

private slots:
    void workspaceSelected(int idx);
    void updateWorkspaces();

public:
    WorkspaceComboBox(MuseScore*, QWidget* parent = nullptr);

    void retranslate();
};
} // namespace Ms

#endif
