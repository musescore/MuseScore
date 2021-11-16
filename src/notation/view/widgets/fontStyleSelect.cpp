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

#include "ui/view/widgetutils.h"

using namespace mu::notation;
using namespace mu::ui;

FontStyleSelect::FontStyleSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    WidgetUtils::setWidgetIcon(bold, IconCode::Code::TEXT_BOLD);
    WidgetUtils::setWidgetIcon(italic, IconCode::Code::TEXT_ITALIC);
    WidgetUtils::setWidgetIcon(underline, IconCode::Code::TEXT_UNDERLINE);
    WidgetUtils::setWidgetIcon(strike, IconCode::Code::TEXT_STRIKE);

    connect(bold, &QPushButton::toggled, this, &FontStyleSelect::_fontStyleChanged);
    connect(italic, &QPushButton::toggled, this, &FontStyleSelect::_fontStyleChanged);
    connect(underline, &QPushButton::toggled, this, &FontStyleSelect::_fontStyleChanged);
    connect(strike, &QPushButton::toggled, this, &FontStyleSelect::_fontStyleChanged);
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
    if (strike->isChecked()) {
        fs = fs + Ms::FontStyle::Strike;
    }

    return fs;
}

void FontStyleSelect::setFontStyle(Ms::FontStyle fs)
{
    bold->setChecked(fs & Ms::FontStyle::Bold);
    italic->setChecked(fs & Ms::FontStyle::Italic);
    underline->setChecked(fs & Ms::FontStyle::Underline);
    strike->setChecked(fs & Ms::FontStyle::Strike);
}
