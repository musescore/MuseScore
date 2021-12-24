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

#include "alignSelect.h"

#include <QButtonGroup>

#include "ui/view/widgetutils.h"

using namespace mu::notation;
using namespace mu::ui;

AlignSelect::AlignSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    g1 = new QButtonGroup(this);
    g1->addButton(alignLeft);
    g1->addButton(alignHCenter);
    g1->addButton(alignRight);

    g2 = new QButtonGroup(this);
    g2->addButton(alignTop);
    g2->addButton(alignVCenter);
    g2->addButton(alignBaseline);
    g2->addButton(alignBottom);

    WidgetUtils::setWidgetIcon(alignLeft, IconCode::Code::TEXT_ALIGN_LEFT);
    WidgetUtils::setWidgetIcon(alignRight, IconCode::Code::TEXT_ALIGN_RIGHT);
    WidgetUtils::setWidgetIcon(alignHCenter, IconCode::Code::TEXT_ALIGN_CENTER);
    WidgetUtils::setWidgetIcon(alignVCenter, IconCode::Code::TEXT_ALIGN_MIDDLE);
    WidgetUtils::setWidgetIcon(alignTop, IconCode::Code::TEXT_ALIGN_ABOVE);
    WidgetUtils::setWidgetIcon(alignBaseline, IconCode::Code::TEXT_ALIGN_BASELINE);
    WidgetUtils::setWidgetIcon(alignBottom, IconCode::Code::TEXT_ALIGN_UNDER);

    connect(g1, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
            this, &AlignSelect::_alignChanged);
    connect(g2, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
            this, &AlignSelect::_alignChanged);
}

void AlignSelect::_alignChanged()
{
    emit alignChanged(align());
}

Ms::Align AlignSelect::align() const
{
    Ms::Align a = { Ms::AlignH::LEFT, Ms::AlignV::TOP };
    if (alignHCenter->isChecked()) {
        a = Ms::AlignH::HCENTER;
    } else if (alignRight->isChecked()) {
        a = Ms::AlignH::RIGHT;
    }
    if (alignVCenter->isChecked()) {
        a = Ms::AlignV::VCENTER;
    } else if (alignBottom->isChecked()) {
        a = Ms::AlignV::BOTTOM;
    } else if (alignBaseline->isChecked()) {
        a = Ms::AlignV::BASELINE;
    }
    return a;
}

void AlignSelect::setAlign(Ms::Align a)
{
    blockAlign(true);
    if (a == Ms::AlignH::HCENTER) {
        alignHCenter->setChecked(true);
    } else if (a == Ms::AlignH::RIGHT) {
        alignRight->setChecked(true);
    } else {
        alignLeft->setChecked(true);
    }
    if (a == Ms::AlignV::VCENTER) {
        alignVCenter->setChecked(true);
    } else if (a == Ms::AlignV::BOTTOM) {
        alignBottom->setChecked(true);
    } else if (a == Ms::AlignV::BASELINE) {
        alignBaseline->setChecked(true);
    } else {
        alignTop->setChecked(true);
    }
    blockAlign(false);
}

void AlignSelect::blockAlign(bool val)
{
    g1->blockSignals(val);
    g2->blockSignals(val);
}
