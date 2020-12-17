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

#include "fontStyleSelect.h"

#include "ui/view/iconcodes.h"

using namespace mu::framework;
using namespace mu::notation;

//---------------------------------------------------------
//    FontStyleSelect
//---------------------------------------------------------

FontStyleSelect::FontStyleSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    bold->setText(iconCodeToChar(IconCode::Code::TEXT_BOLD));
    italic->setText(iconCodeToChar(IconCode::Code::TEXT_ITALIC));
    underline->setText(iconCodeToChar(IconCode::Code::TEXT_UNDERLINE));

    connect(bold, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
    connect(italic, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
    connect(underline, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
}

//---------------------------------------------------------
//   _fontStyleChanged
//---------------------------------------------------------

void FontStyleSelect::_fontStyleChanged()
{
    emit fontStyleChanged(fontStyle());
}

//---------------------------------------------------------
//   fontStyle
//---------------------------------------------------------

Ms::FontStyle FontStyleSelect::fontStyle() const
{
    Ms::FontStyle fs = Ms::FontStyle::Normal;

    if (bold->isChecked()) {
        fs = fs + Ms::FontStyle::Bold;
    }
    if (italic->isChecked()) {
        fs = fs + Ms::FontStyle::Italic;
    }
    if (underline->isChecked()) {
        fs = fs + Ms::FontStyle::Underline;
    }

    return fs;
}

//---------------------------------------------------------
//   setFontStyle
//---------------------------------------------------------

void FontStyleSelect::setFontStyle(Ms::FontStyle fs)
{
    bold->setChecked(fs & Ms::FontStyle::Bold);
    italic->setChecked(fs & Ms::FontStyle::Italic);
    underline->setChecked(fs & Ms::FontStyle::Underline);
}
