//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include "createpalettedialog.h"

#include "ui_createPalette.h"

namespace Ms {
CreatePaletteDialog::CreatePaletteDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::CreatePaletteDialog)
{
    ui->setupUi(this);

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    Q_ASSERT(okButton);

    okButton->setText(tr("Create"));
    okButton->setEnabled(!ui->paletteNameLineEdit->text().isEmpty());

    connect(ui->paletteNameLineEdit, &QLineEdit::textChanged, okButton, [okButton](const QString& text) {
            okButton->setEnabled(!text.isEmpty());
        });
}

CreatePaletteDialog::~CreatePaletteDialog()
{
    delete ui;
}

QString CreatePaletteDialog::paletteName() const
{
    return ui->paletteNameLineEdit->text();
}
} // namespace Ms
