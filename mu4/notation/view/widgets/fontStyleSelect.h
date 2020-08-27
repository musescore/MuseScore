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
#ifndef MU_NOTATION_FONTSTYLESELECT_H
#define MU_NOTATION_FONTSTYLESELECT_H

#include "ui_font_style_select.h"
#include "libmscore/types.h"

namespace mu {
namespace notation {
//---------------------------------------------------------
//   FontStyleSelect
//---------------------------------------------------------

class FontStyleSelect : public QWidget, public Ui::FontStyleSelect
{
    Q_OBJECT

private slots:
    void _fontStyleChanged();

signals:
    void fontStyleChanged(Ms::FontStyle);

public:
    FontStyleSelect(QWidget* parent);
    Ms::FontStyle fontStyle() const;
    void setFontStyle(Ms::FontStyle);
};
}
}

#endif // MU_NOTATION_FONTSTYLESELECT_H
