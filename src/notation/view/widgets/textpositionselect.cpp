/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "textpositionselect.h"

#include <QButtonGroup>

#include "ui/view/widgetutils.h"

using namespace mu::notation;
using namespace muse::ui;

TextPositionSelect::TextPositionSelect(QWidget* parent)
    : QWidget(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setupUi(this);

    positionButtons = new QButtonGroup(this);
    positionButtons->addButton(positionLeft);
    positionButtons->addButton(positionHCenter);
    positionButtons->addButton(positionRight);

    WidgetUtils::setWidgetIcon(positionLeft, IconCode::Code::ALIGN_LEFT);
    WidgetUtils::setWidgetIcon(positionRight, IconCode::Code::ALIGN_RIGHT);
    WidgetUtils::setWidgetIcon(positionHCenter, IconCode::Code::ALIGN_HORIZONTAL_CENTER);

    connect(positionButtons, &QButtonGroup::buttonToggled, this, &TextPositionSelect::_positionChanged);
    connect(positionButtons, &QButtonGroup::buttonToggled, this, &TextPositionSelect::_positionChanged);
}

void TextPositionSelect::_positionChanged()
{
    emit positionChanged(position());
}

mu::engraving::AlignH TextPositionSelect::position() const
{
    mu::engraving::AlignH position = mu::engraving::AlignH::LEFT;
    if (positionHCenter->isChecked()) {
        position = mu::engraving::AlignH::HCENTER;
    } else if (positionRight->isChecked()) {
        position = mu::engraving::AlignH::RIGHT;
    }
    return position;
}

void TextPositionSelect::setPosition(mu::engraving::AlignH a)
{
    blockPosition(true);
    if (a == mu::engraving::AlignH::HCENTER) {
        positionHCenter->setChecked(true);
    } else if (a == mu::engraving::AlignH::RIGHT) {
        positionRight->setChecked(true);
    } else {
        positionLeft->setChecked(true);
    }
    blockPosition(false);
}

void TextPositionSelect::blockPosition(bool val)
{
    positionButtons->blockSignals(val);
}
