/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
using namespace muse::ui;

FontStyleSelect::FontStyleSelect(QWidget* parent)
    : QWidget(parent), muse::Injectable(muse::iocCtxForQWidget(this))
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

mu::engraving::FontStyle FontStyleSelect::fontStyle() const
{
    mu::engraving::FontStyle fs = mu::engraving::FontStyle::Normal;

    if (bold->isChecked()) {
        fs = fs + mu::engraving::FontStyle::Bold;
    }
    if (italic->isChecked()) {
        fs = fs + mu::engraving::FontStyle::Italic;
    }
    if (underline->isChecked()) {
        fs = fs + mu::engraving::FontStyle::Underline;
    }
    if (strike->isChecked()) {
        fs = fs + mu::engraving::FontStyle::Strike;
    }

    return fs;
}

void FontStyleSelect::setFontStyle(mu::engraving::FontStyle fs)
{
    bold->setChecked(fs & mu::engraving::FontStyle::Bold);
    italic->setChecked(fs & mu::engraving::FontStyle::Italic);
    underline->setChecked(fs & mu::engraving::FontStyle::Underline);
    strike->setChecked(fs & mu::engraving::FontStyle::Strike);
}
