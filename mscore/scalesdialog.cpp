#include "scalesdialog.h"
#include "ui_scalesdialog.h"
#include "editfifthsdialog.h"
#include "editnotesdialog.h"

namespace Ms {

//---------------------------------------------------------
//   ScalesDialog c'tor
//---------------------------------------------------------

ScalesDialog::ScalesDialog(Scale scale, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::ScalesDialog)
      {
      ui->setupUi(this);

      mscore = static_cast<MuseScore*>(parent);
      this->scale = scale;
      showData();

      connect(ui->pushEditFifths, SIGNAL(clicked()), this, SLOT(editAsFifths()));
      connect(ui->pushEditNotes, SIGNAL(clicked()), this, SLOT(editAsNotes()));
      connect(ui->pushRestore, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
      connect(ui->pushImport, SIGNAL(clicked()), this, SLOT(importScalaFile()));
      connect(ui->scaleName, SIGNAL(editingFinished()), this, SLOT(updateScaleName()));
      connect(ui->checkUpdatePitches, SIGNAL(stateChanged()), this, SLOT(updatePitches()));
      }

//---------------------------------------------------------
//   ScalesDialog d'tor
//---------------------------------------------------------
ScalesDialog::~ScalesDialog()
      {
      delete ui;
      }

//---------------------------------------------------------
//   editAsFifths
//---------------------------------------------------------

void ScalesDialog::editAsFifths()
      {
      EditFifthsDialog editDialog(scale, this);

      editDialog.setModal(true);
      editDialog.exec();

      scale = editDialog.getScale();
      }

//---------------------------------------------------------
//   editAsNotes
//---------------------------------------------------------

void ScalesDialog::editAsNotes()
      {
      EditNotesDialog editDialog(scale, this);

      editDialog.setModal(true);
      editDialog.exec();

      scale = editDialog.getScale();
      }

//---------------------------------------------------------
//   showData
//---------------------------------------------------------

void ScalesDialog::showData()
      {
      showScaleName();
      if (scale.getUpdatePitches())
            ui->checkUpdatePitches->setCheckState(Qt::Checked);
      else
            ui->checkUpdatePitches->setCheckState(Qt::Unchecked);
      }

//---------------------------------------------------------
//   showScaleName
//---------------------------------------------------------

void ScalesDialog::showScaleName()
      {
      ui->scaleName->setText(scale.getName());
      }

//---------------------------------------------------------
//   updateScaleName
//---------------------------------------------------------

void ScalesDialog::updateScaleName()
      {
      scale.setName(ui->scaleName->text());
      }

//---------------------------------------------------------
//   restoreDefaults
//---------------------------------------------------------

void ScalesDialog::restoreDefaults()
      {
      scale = Scale();
      showData();
      }

//---------------------------------------------------------
//   importScalaFile
//---------------------------------------------------------

void ScalesDialog::importScalaFile()
      {
      QString name = mscore->getScalaFilename();
      Scale s;
      if (name.isEmpty())
            return;

      if (!s.loadScalaFile(name))
            QMessageBox::critical(this, tr("MuseScore: Load Scala File"), tr("Some error"));
            //MScore::lastError
      else {
            scale = s;
            showData();
            }
      }

//---------------------------------------------------------
//   updatePitches
//---------------------------------------------------------

void ScalesDialog::updatePitches()
      {
      scale.setUpdatePitches(ui->checkUpdatePitches->checkState() == Qt::Checked);
      }

};