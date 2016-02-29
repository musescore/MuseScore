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

      connect(ui->radioAFreq, SIGNAL(clicked()), this, SLOT(aFreqClicked()));
      connect(ui->radioACent, SIGNAL(clicked()), this, SLOT(aCentsClicked()));

      connect(ui->scaleName, SIGNAL(editingFinished()), this, SLOT(updateScaleName()));
      connect(ui->lineCents, SIGNAL(editingFinished()), this, SLOT(updateAtuning()));
      connect(ui->lineFreqFrom, SIGNAL(editingFinished()), this, SLOT(updateAtuning()));
      connect(ui->lineFreqTo, SIGNAL(editingFinished()), this, SLOT(updateAtuning()));
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
      showAtuning();
      }

//---------------------------------------------------------
//   showAtuning
//---------------------------------------------------------

void ScalesDialog::showAtuning()
      {
      QString Atuning = scale.getAtuning();
      if (Atuning.contains('/')) {
            QStringList values = Atuning.split('/');
            ui->radioAFreq->setChecked(true);
            ui->radioACent->setChecked(false);
            ui->lineCents->setEnabled(false);
            ui->lineFreqFrom->setEnabled(true);
            ui->lineFreqTo->setEnabled(true);

            ui->lineFreqFrom->setText(values[0]);
            ui->lineFreqTo->setText(values[1]);
            ui->lineCents->setText("0");
            }
      else {
            ui->radioAFreq->setChecked(false);
            ui->radioACent->setChecked(true);
            ui->lineCents->setEnabled(true);
            ui->lineFreqFrom->setEnabled(false);
            ui->lineFreqTo->setEnabled(false);

            ui->lineFreqFrom->setText("440");
            ui->lineFreqTo->setText("440");
            ui->lineCents->setText(Atuning);
            }
      }

//---------------------------------------------------------
//   updateAtuning
//---------------------------------------------------------

void ScalesDialog::updateAtuning()
      {
      if (ui->radioAFreq->isDown())
            scale.setAtuning(ui->lineFreqFrom->text() + "/" + ui->lineFreqTo->text());
      else
            scale.setAtuning(ui->lineCents->text());
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
//   aFreqClicked
//---------------------------------------------------------

void ScalesDialog::aFreqClicked()
      {
      if (ui->lineFreqFrom->isEnabled())
            return;
      scale.setAtuning(QString("440/440"));
      showAtuning();
      }

//---------------------------------------------------------
//   aCentsClicked
//---------------------------------------------------------

void ScalesDialog::aCentsClicked()
      {
      if (ui->lineCents->isEnabled())
            return;
      scale.setAtuning(QString("0"));
      showAtuning();
      }

};