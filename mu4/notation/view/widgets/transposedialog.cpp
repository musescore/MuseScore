//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "transposedialog.h"

#include "framework/global/widgetstatestore.h"

using namespace Ms;
using namespace mu::notation;

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("TransposeDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(transposeByKey,      &QGroupBox::clicked, this, &TransposeDialog::transposeByKeyToggled);
    connect(transposeByInterval, &QGroupBox::clicked, this, &TransposeDialog::transposeByIntervalToggled);
    connect(chromaticBox,        &QGroupBox::clicked, this, &TransposeDialog::chromaticBoxToggled);
    connect(diatonicBox,         &QGroupBox::clicked, this, &TransposeDialog::diatonicBoxToggled);

    WidgetStateStore::restoreGeometry(this);
}

TransposeDialog::TransposeDialog(const TransposeDialog& dialog)
    : TransposeDialog(dialog.parentWidget())
{
}

//---------------------------------------------------------
//   TransposeDialog slots
//---------------------------------------------------------

void TransposeDialog::transposeByKeyToggled(bool val)
{
    transposeByInterval->setChecked(!val);
}

void TransposeDialog::transposeByIntervalToggled(bool val)
{
    transposeByKey->setChecked(!val);
}

void TransposeDialog::chromaticBoxToggled(bool val)
{
    diatonicBox->setChecked(!val);
}

void TransposeDialog::diatonicBoxToggled(bool val)
{
    chromaticBox->setChecked(!val);
}

//---------------------------------------------------------
//   mode
//---------------------------------------------------------

TransposeMode TransposeDialog::mode() const
{
    return chromaticBox->isChecked()
           ? (transposeByKey->isChecked() ? TransposeMode::TO_KEY : TransposeMode::BY_INTERVAL)
           : TransposeMode::DIATONICALLY;
}

//---------------------------------------------------------
//   enableTransposeByKey
//---------------------------------------------------------

void TransposeDialog::enableTransposeToKey(bool val)
{
    transposeByKey->setEnabled(val);
    transposeByInterval->setChecked(!val);
    transposeByKey->setChecked(val);
}

//---------------------------------------------------------
//   enableTransposeChordNames
//---------------------------------------------------------

void TransposeDialog::enableTransposeChordNames(bool val)
{
    transposeChordNames->setEnabled(val);
    transposeChordNames->setChecked(val);
}

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

TransposeDirection TransposeDialog::direction() const
{
    switch (mode()) {
    case TransposeMode::TO_KEY:
        if (closestKey->isChecked()) {
            return TransposeDirection::CLOSEST;
        }
        return upKey->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    case TransposeMode::BY_INTERVAL:
        return upInterval->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    case TransposeMode::DIATONICALLY:
        return upDiatonic->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    }
    return TransposeDirection::UP;
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TransposeDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}
