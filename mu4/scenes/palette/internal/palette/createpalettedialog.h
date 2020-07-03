//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#ifndef __CREATEPALETTEDIALOG_H__
#define __CREATEPALETTEDIALOG_H__

namespace Ui {
class CreatePaletteDialog;
}

namespace Ms {
//---------------------------------------------------------
//   CreatePaletteDialog
//---------------------------------------------------------

class CreatePaletteDialog : public QDialog
{
    Q_OBJECT

    Ui::CreatePaletteDialog* ui;

public:
    CreatePaletteDialog(QWidget* parent = 0);
    ~CreatePaletteDialog();

    QString paletteName() const;
};
} // namespace Ms

#endif //__PALETTECELLDIALOGS_H__
