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

using mu::engraving::FontStyle;

FontStyleSelect::FontStyleSelect(QWidget* parent)
    : QWidget(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setupUi(this);

    WidgetUtils::setWidgetIcon(bold, IconCode::Code::TEXT_BOLD);
    WidgetUtils::setWidgetIcon(italic, IconCode::Code::TEXT_ITALIC);
    WidgetUtils::setWidgetIcon(underline, IconCode::Code::TEXT_UNDERLINE);
    WidgetUtils::setWidgetIcon(strike, IconCode::Code::TEXT_STRIKE);

    connect(bold, &QPushButton::toggled, this, &FontStyleSelect::onFontStyleChanged);
    connect(italic, &QPushButton::toggled, this, &FontStyleSelect::onFontStyleChanged);
    connect(underline, &QPushButton::toggled, this, &FontStyleSelect::onFontStyleChanged);
    connect(strike, &QPushButton::toggled, this, &FontStyleSelect::onFontStyleChanged);
}

void FontStyleSelect::onFontStyleChanged()
{
    emit fontStyleChanged(fontStyle());
}

FontStyle FontStyleSelect::fontStyle() const
{
    FontStyle fs = FontStyle::Normal;

    if (bold->isChecked()) {
        fs = fs + FontStyle::Bold;
    }
    if (italic->isChecked()) {
        fs = fs + FontStyle::Italic;
    }
    if (underline->isChecked()) {
        fs = fs + FontStyle::Underline;
    }
    if (strike->isChecked()) {
        fs = fs + FontStyle::Strike;
    }

    return fs;
}

void FontStyleSelect::setFontStyle(FontStyle fs)
{
    bold->setChecked(fs & FontStyle::Bold);
    italic->setChecked(fs & FontStyle::Italic);
    underline->setChecked(fs & FontStyle::Underline);
    strike->setChecked(fs & FontStyle::Strike);
}
