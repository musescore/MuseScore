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

#include "palettedialog.h"

#include "musescore.h"
#include "palettetree.h"
#include "ui_paletteProperties.h"

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

      Q_ASSERT(p);
      fillControlsWithData();
      setInitialProperties();
      
      connect(ui->showGrid, &QCheckBox::stateChanged, this, &PalettePropertiesDialog::gridCheckBoxChanged);
      connect(ui->name, &QLineEdit::textEdited, this, &PalettePropertiesDialog::nameChanged);
      connect(ui->cellHeight, QOverload<int>::of(&QSpinBox::valueChanged), this, &PalettePropertiesDialog::heightChanged);
      connect(ui->cellWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &PalettePropertiesDialog::widthChanged);
      connect(ui->elementOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PalettePropertiesDialog::offsetChanged);
      connect(ui->mag, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PalettePropertiesDialog::scaleChanged);
      
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
//   PalettePropertiesDialog::fillControlsWithData
//---------------------------------------------------------

void PalettePropertiesDialog::fillControlsWithData()
      {
      ui->name->setText(palette->translatedName());
      const QSize grid = palette->gridSize();
      ui->cellWidth->setValue(grid.width());
      ui->cellHeight->setValue(grid.height());
      ui->showGrid->setChecked(palette->drawGrid());
      ui->elementOffset->setValue(palette->yOffset());
      ui->mag->setValue(palette->mag());

      ui->elementOffset->setEnabled(false); // not enabled currently (need to allow a parameter in PaletteCellIconEngine)
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PalettePropertiesDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

void PalettePropertiesDialog::gridCheckBoxChanged(int state)
      {
      isGridCheckBoxChanged = (state != gridCheckboxInitialState);
      palette->setDrawGrid(ui->showGrid->isChecked());
      emit changed();
      }
      
void PalettePropertiesDialog::nameChanged(const QString &text)
      {
      isNameChanged = (text != initialTranslatedName);
      palette->setName(isNameChanged ? ui->name->text() : initialName); // preserve old name if possible to keep translations
      emit changed();
      }
      
void PalettePropertiesDialog::heightChanged(int height)
      {
      isHeightChanged = (height != initialHeight);
      palette->setGrid(ui->cellWidth->value(), height);
      emit changed();
      }
      
void PalettePropertiesDialog::widthChanged(int width)
      {
      isWidthChanged = (width != initialWidth);
      palette->setGrid(width, ui->cellHeight->value());
      emit changed();
      }
      
void PalettePropertiesDialog::offsetChanged(double offset)
      {
      //see https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare to clarify using 1.f below
      isOffsetChanged = !qFuzzyCompare((1.f + offset), (1.f + initialOffset));
      palette->setYOffset(offset);
      emit changed();
      }
      
void PalettePropertiesDialog::scaleChanged(double scale)
      {
      //see https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare to clarify using 1.f below
      isScaleChanged = !qFuzzyCompare((1.f + scale), (1.f + initialScale));
      palette->setMag(scale);
      emit changed();
      }
/*
/  PalettePropertiesDialog::reject()
/      Shows additional dialog box asking a user whether he wants to reject the changes made
*/
void PalettePropertiesDialog::reject()
      {
      //if nothing is changed, just close the palette properties dialog
      if (areInitialPropertiesChanged()) {
            applyInitialPropertiesToThePalette();
            QDialog::reject();
            return;
            }
      
      QMessageBox msgBox;
      msgBox.setText(tr("The palette properties have been modified."));
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
      
void PalettePropertiesDialog::setInitialProperties()
      {
      gridCheckboxInitialState = palette->drawGrid() ? Qt::Checked : Qt::Unchecked;
      initialName = palette->name();
      initialTranslatedName = palette->translatedName();
      initialWidth = palette->gridSize().width();
      initialHeight = palette->gridSize().height();
      initialOffset = palette->yOffset();
      initialScale = palette->mag();
      }

bool PalettePropertiesDialog::areInitialPropertiesChanged() const
      {
      return !isGridCheckBoxChanged && !isNameChanged && !isHeightChanged && !isWidthChanged && !isOffsetChanged && !isScaleChanged;
      }
      
void PalettePropertiesDialog::applyInitialPropertiesToThePalette()
      {
      Q_ASSERT(palette);
      
      palette->setDrawGrid(gridCheckboxInitialState);
      palette->setName(initialName);
      palette->setGrid(initialWidth, initialHeight);
      palette->setYOffset(initialOffset);
      palette->setMag(initialScale);
      }
      
} // namespace Ms
