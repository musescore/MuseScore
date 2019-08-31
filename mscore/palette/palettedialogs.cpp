//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "palettedialogs.h"

#include "musescore.h"
#include "palettetree.h"
#include "ui_palette.h"

namespace Ms {

//---------------------------------------------------------
//   PalettePropertiesDialog
//---------------------------------------------------------

PalettePropertiesDialog::PalettePropertiesDialog(PalettePanel* p, QWidget* parent)
   : QDialog(parent), ui(new Ui::PaletteProperties), palette(p)
      {
      setObjectName("PalettePropertiesDialog");
      ui->setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      if (p)
            setData(p);

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   ~PalettePropertiesDialog
//---------------------------------------------------------

PalettePropertiesDialog::~PalettePropertiesDialog()
      {
      delete ui;
      }

//---------------------------------------------------------
//   PalettePropertiesDialog::setData
//---------------------------------------------------------

void PalettePropertiesDialog::setData(const PalettePanel* p)
      {
      ui->name->setText(p->name());
      const QSize grid = p->gridSize();
      ui->cellWidth->setValue(grid.width());
      ui->cellHeight->setValue(grid.height());
      ui->showGrid->setChecked(p->drawGrid());
      ui->moreElements->setChecked(p->moreElements());
      ui->elementOffset->setValue(p->yOffset());
      ui->mag->setValue(p->mag());

      ui->elementOffset->setEnabled(false); // not enabled currently (need to allow a parameter in PaletteCellIconEngine)
      ui->moreElements->setEnabled(false); // maybe we remove this setting at all
      }

//---------------------------------------------------------
//   PalettePropertiesDialog::accept
//---------------------------------------------------------

void PalettePropertiesDialog::accept()
      {
      if (!palette)
            return;

      if (ui->name->text() != palette->name())
            palette->setName(ui->name->text());

      palette->setGrid(ui->cellWidth->value(), ui->cellHeight->value());
      palette->setDrawGrid(ui->showGrid->isChecked());
      palette->setMoreElements(ui->moreElements->isChecked());
      palette->setYOffset(ui->elementOffset->value());
      palette->setMag(ui->mag->value());

      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PalettePropertiesDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

} // namespace Ms
