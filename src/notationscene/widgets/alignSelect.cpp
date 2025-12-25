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

#include "alignSelect.h"

#include <QButtonGroup>

#include "ui/view/widgetutils.h"

using namespace mu::notation;
using namespace muse::ui;

AlignSelect::AlignSelect(QWidget* parent)
    : QWidget(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setupUi(this);

    horizontalAlignButtons = new QButtonGroup(this);
    horizontalAlignButtons->addButton(alignLeft);
    horizontalAlignButtons->addButton(alignHCenter);
    horizontalAlignButtons->addButton(alignRight);
    horizontalAlignButtons->addButton(alignJustify);

    verticalAlignButtons = new QButtonGroup(this);
    verticalAlignButtons->addButton(alignTop);
    verticalAlignButtons->addButton(alignVCenter);
    verticalAlignButtons->addButton(alignBaseline);
    verticalAlignButtons->addButton(alignBottom);

    positionButtons = new QButtonGroup(this);
    positionButtons->addButton(positionLeft);
    positionButtons->addButton(positionHCenter);
    positionButtons->addButton(positionRight);

    WidgetUtils::setWidgetIcon(alignLeft, IconCode::Code::TEXT_ALIGN_LEFT);
    WidgetUtils::setWidgetIcon(alignRight, IconCode::Code::TEXT_ALIGN_RIGHT);
    WidgetUtils::setWidgetIcon(alignHCenter, IconCode::Code::TEXT_ALIGN_CENTER);
    WidgetUtils::setWidgetIcon(alignJustify, IconCode::Code::TEXT_ALIGN_JUSTIFY);
    WidgetUtils::setWidgetIcon(alignVCenter, IconCode::Code::TEXT_ALIGN_MIDDLE);
    WidgetUtils::setWidgetIcon(alignTop, IconCode::Code::TEXT_ALIGN_TOP);
    WidgetUtils::setWidgetIcon(alignBaseline, IconCode::Code::TEXT_ALIGN_BASELINE);
    WidgetUtils::setWidgetIcon(alignBottom, IconCode::Code::TEXT_ALIGN_BOTTOM);
    WidgetUtils::setWidgetIcon(positionLeft, IconCode::Code::ALIGN_LEFT);
    WidgetUtils::setWidgetIcon(positionRight, IconCode::Code::ALIGN_RIGHT);
    WidgetUtils::setWidgetIcon(positionHCenter, IconCode::Code::ALIGN_HORIZONTAL_CENTER);

    connect(horizontalAlignButtons, &QButtonGroup::buttonToggled, this, &AlignSelect::_alignChanged);
    connect(verticalAlignButtons, &QButtonGroup::buttonToggled, this, &AlignSelect::_alignChanged);
    connect(positionButtons, &QButtonGroup::buttonToggled, this, &AlignSelect::_positionChanged);
}

void AlignSelect::_alignChanged()
{
    emit alignChanged(align());
}

void AlignSelect::_positionChanged()
{
    emit positionChanged(position());
}

mu::engraving::Align AlignSelect::align() const
{
    mu::engraving::Align a = { mu::engraving::AlignH::LEFT, mu::engraving::AlignV::TOP };
    if (alignHCenter->isChecked()) {
        a = mu::engraving::AlignH::HCENTER;
    } else if (alignRight->isChecked()) {
        a = mu::engraving::AlignH::RIGHT;
    } else if (alignJustify->isChecked()) {
        a = mu::engraving::AlignH::JUSTIFY;
    }
    if (alignVCenter->isChecked()) {
        a = mu::engraving::AlignV::VCENTER;
    } else if (alignBottom->isChecked()) {
        a = mu::engraving::AlignV::BOTTOM;
    } else if (alignBaseline->isChecked()) {
        a = mu::engraving::AlignV::BASELINE;
    }
    return a;
}

mu::engraving::AlignH AlignSelect::position() const
{
    mu::engraving::AlignH position = mu::engraving::AlignH::LEFT;
    if (positionHCenter->isChecked()) {
        position = mu::engraving::AlignH::HCENTER;
    } else if (positionRight->isChecked()) {
        position = mu::engraving::AlignH::RIGHT;
    }
    return position;
}

void AlignSelect::setAlign(mu::engraving::Align a)
{
    blockAlign(true);
    if (a == mu::engraving::AlignH::HCENTER) {
        alignHCenter->setChecked(true);
    } else if (a == mu::engraving::AlignH::RIGHT) {
        alignRight->setChecked(true);
    } else if (a == mu::engraving::AlignH::JUSTIFY) {
        alignJustify->setChecked(true);
    } else {
        alignLeft->setChecked(true);
    }
    if (a == mu::engraving::AlignV::VCENTER) {
        alignVCenter->setChecked(true);
    } else if (a == mu::engraving::AlignV::BOTTOM) {
        alignBottom->setChecked(true);
    } else if (a == mu::engraving::AlignV::BASELINE) {
        alignBaseline->setChecked(true);
    } else {
        alignTop->setChecked(true);
    }
    blockAlign(false);
}

void AlignSelect::setPosition(mu::engraving::AlignH a)
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

void AlignSelect::blockAlign(bool val)
{
    horizontalAlignButtons->blockSignals(val);
    verticalAlignButtons->blockSignals(val);
}

void AlignSelect::blockPosition(bool val)
{
    positionButtons->blockSignals(val);
}
