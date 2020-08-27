//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#include "palettecelldialog.h"

#include "palettetree.h"
#include "ui_paletteCellProperties.h"

#include "widgetstatestore.h"

namespace Ms {
PaletteCellPropertiesDialog::PaletteCellPropertiesDialog(PaletteCell* p, QWidget* parent)
    : QDialog(parent), ui(new Ui::PaletteCellProperties), cell(p)
{
    setObjectName("PaletteCellProperties");
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    fillControlsWithData();
    setInitialProperties();

    connect(ui->drawStaff, &QCheckBox::stateChanged, this, &PaletteCellPropertiesDialog::drawStaffCheckBoxChanged);
    connect(ui->name, &QLineEdit::textEdited, this, &PaletteCellPropertiesDialog::nameChanged);
    connect(ui->xoffset, QOverload<double>::of(
                &QDoubleSpinBox::valueChanged), this, &PaletteCellPropertiesDialog::xOffsetChanged);
    connect(ui->yoffset, QOverload<double>::of(
                &QDoubleSpinBox::valueChanged), this, &PaletteCellPropertiesDialog::yOffsetChanged);
    connect(ui->scale, QOverload<double>::of(
                &QDoubleSpinBox::valueChanged), this, &PaletteCellPropertiesDialog::scaleChanged);

    WidgetStateStore::restoreGeometry(this);
}

void PaletteCellPropertiesDialog::fillControlsWithData()
{
    ui->xoffset->setValue(cell->xoffset);
    ui->yoffset->setValue(cell->yoffset);
    ui->scale->setValue(cell->mag);
    ui->drawStaff->setChecked(cell->drawStaff);
    ui->name->setText(cell->translatedName());
}

PaletteCellPropertiesDialog::~PaletteCellPropertiesDialog()
{
    delete ui;
}

void PaletteCellPropertiesDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

void PaletteCellPropertiesDialog::drawStaffCheckBoxChanged(int state)
{
    isDrawStaffCheckBoxChanged = (state != drawStaffCheckboxInitialState);
    cell->drawStaff = ui->drawStaff->isChecked();
    cell->custom = true;
    emit changed();
}

void PaletteCellPropertiesDialog::nameChanged(const QString& text)
{
    isNameChanged = (text != initialTranslatedName);
    cell->name = isNameChanged ? text : initialName;   // preserve old name if possible to keep translations
    // don't mark cell custom if only its name gets changed
    emit changed();
}

void PaletteCellPropertiesDialog::xOffsetChanged(double xOffset)
{
    //see https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare to clarify using 1.f below
    isXOffsetChanged = !qFuzzyCompare((1.f + xOffset), (1.f + initialXOffset));
    cell->xoffset = xOffset;
    cell->custom = true;
    emit changed();
}

void PaletteCellPropertiesDialog::yOffsetChanged(double yOffset)
{
    //see https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare to clarify using 1.f below
    isYOffsetChanged = !qFuzzyCompare((1.f + yOffset), (1.f + initialYOffset));
    cell->yoffset = yOffset;
    cell->custom = true;
    emit changed();
}

void PaletteCellPropertiesDialog::scaleChanged(double scale)
{
    //see https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare to clarify using 1.f below
    isScaleChanged = !qFuzzyCompare((1.f + scale), (1.f + initialScale));
    cell->mag = scale;
    cell->custom = true;
    emit changed();
}

/*
/  PaletteCellProperties::reject()
/      Shows additional dialog box asking a user whether he wants to reject the changes made
*/
void PaletteCellPropertiesDialog::reject()
{
    //if nothing is changed, just close the palette properties dialog
    if (areInitialPropertiesChanged()) {
        applyInitialPropertiesToThePalette();
        QDialog::reject();
        return;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("The palette cell properties have been modified."));
    msgBox.setInformativeText(tr("Do you want to save your changes?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Discard);
    msgBox.setIcon(QMessageBox::Question);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
        QDialog::accept();
        break;
    case QMessageBox::Discard:
        applyInitialPropertiesToThePalette();
        QDialog::reject();
        break;
    case QMessageBox::Cancel:
        break;
    default:
        Q_ASSERT(false && "Should never be reached");
    }
}

void PaletteCellPropertiesDialog::setInitialProperties()
{
    drawStaffCheckboxInitialState = cell->drawStaff ? Qt::Checked : Qt::Unchecked;
    initialName = cell->name;
    initialTranslatedName = cell->translatedName();
    initialYOffset = cell->yoffset;
    initialXOffset = cell->xoffset;
    initialScale = cell->mag;
    initialCustomState = cell->custom;
}

bool PaletteCellPropertiesDialog::areInitialPropertiesChanged() const
{
    return !isDrawStaffCheckBoxChanged && !isNameChanged && !isXOffsetChanged && !isYOffsetChanged && !isScaleChanged;
}

void PaletteCellPropertiesDialog::applyInitialPropertiesToThePalette()
{
    cell->drawStaff = drawStaffCheckboxInitialState;
    cell->name = initialName;
    cell->xoffset = initialXOffset;
    cell->yoffset = initialYOffset;
    cell->mag = initialScale;
    cell->custom = initialCustomState;
}
} // namespace Ms
