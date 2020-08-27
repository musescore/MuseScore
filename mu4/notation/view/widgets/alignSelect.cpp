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

#include "alignSelect.h"

#include "ui/view/iconcodes.h"

using namespace Ms;
using namespace mu::notation;
using namespace mu::framework;

//---------------------------------------------------------
//   AlignSelect
//---------------------------------------------------------

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

    auto iconCodeToChar = [](IconCode::Code code) -> QChar {
                              return QChar(static_cast<char16_t>(code));
                          };

    alignLeft->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_LEFT));
    alignRight->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_RIGHT));
    alignHCenter->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_CENTER));
    alignVCenter->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_MIDDLE));
    alignTop->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_ABOVE));
    alignBaseline->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_BASELINE));
    alignBottom->setText(iconCodeToChar(IconCode::Code::TEXT_ALIGN_UNDER));

    connect(g1, SIGNAL(buttonToggled(int,bool)), SLOT(_alignChanged()));
    connect(g2, SIGNAL(buttonToggled(int,bool)), SLOT(_alignChanged()));
}

///---------------------------------------------------------
//   _alignChanged
//---------------------------------------------------------

void AlignSelect::_alignChanged()
{
    emit alignChanged(align());
}

//---------------------------------------------------------
//   align
//---------------------------------------------------------

Align AlignSelect::align() const
{
    Align a = Align::LEFT;
    if (alignHCenter->isChecked()) {
        a = a | Align::HCENTER;
    } else if (alignRight->isChecked()) {
        a = a | Align::RIGHT;
    }
    if (alignVCenter->isChecked()) {
        a = a | Align::VCENTER;
    } else if (alignBottom->isChecked()) {
        a = a | Align::BOTTOM;
    } else if (alignBaseline->isChecked()) {
        a = a | Align::BASELINE;
    }
    return a;
}

//---------------------------------------------------------
//   blockAlign
//---------------------------------------------------------

void AlignSelect::blockAlign(bool val)
{
    g1->blockSignals(val);
    g2->blockSignals(val);
}

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void AlignSelect::setAlign(Ms::Align a)
{
    blockAlign(true);
    if (a & Align::HCENTER) {
        alignHCenter->setChecked(true);
    } else if (a & Align::RIGHT) {
        alignRight->setChecked(true);
    } else {
        alignLeft->setChecked(true);
    }
    if (a & Align::VCENTER) {
        alignVCenter->setChecked(true);
    } else if (a & Align::BOTTOM) {
        alignBottom->setChecked(true);
    } else if (a & Align::BASELINE) {
        alignBaseline->setChecked(true);
    } else {
        alignTop->setChecked(true);
    }
    blockAlign(false);
}
