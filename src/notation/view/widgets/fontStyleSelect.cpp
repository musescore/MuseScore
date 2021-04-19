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

#include "fontStyleSelect.h"

#include "ui/view/iconcodes.h"

using namespace mu::framework;
using namespace mu::notation;

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

void FontStyleSelect::_fontStyleChanged()
{
    emit fontStyleChanged(fontStyle());
}

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

void FontStyleSelect::setFontStyle(Ms::FontStyle fs)
{
    bold->setChecked(fs & Ms::FontStyle::Bold);
    italic->setChecked(fs & Ms::FontStyle::Italic);
    underline->setChecked(fs & Ms::FontStyle::Underline);
}
