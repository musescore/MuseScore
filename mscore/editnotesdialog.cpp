#include "editnotesdialog.h"
#include "ui_editnotesdialog.h"

namespace Ms {

const int EditNotesDialog::COMBO_SCALE_OPTIONS[Scale::NB_SCALES] = { 7, 12, 17, 21, 35 };

//---------------------------------------------------------
//   EditNotesDialog c'tor
//---------------------------------------------------------

EditNotesDialog::EditNotesDialog(Scale scale, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::EditNotesDialog)
      {
      ui->setupUi(this);

      this->scale = originalScale = scale;
      ui->editNotesLabel->setText(scale.getName());

      this->storingMode = Scale::ABSOLUTE_CENTS;
      QString* originalNotes = scale.getOriginalNotes();
      for (int i = 0; i < TPC_NUM_OF; ++i)
            this->notes[i] = originalNotes[i];
      validator = new QFractionValidator(this);
      validator->setStoringMode(this->storingMode);
      noteEdited = false;

      float aNoteValue = scale.convertNoteValue(notes[TPC_A - TPC_MIN], TPC_A, storingMode, false);
      reference = (aNoteValue == 0);

      initLabelsArray();
      initNoteValuesArray();
      showData();

      connect(ui->checkShowAllNotes, SIGNAL(stateChanged(int)), this, SLOT(showNotes()));
      connect(ui->comboStoringMode, SIGNAL(currentIndexChanged(int)), this, SLOT(showNotes()));
      connect(ui->comboReference, SIGNAL(currentIndexChanged(int)), this, SLOT(showNotes()));

      for (int i = 0; i < TPC_NUM_OF; ++i)
            connect(noteValues[i], SIGNAL(editingFinished()), this, SLOT(noteChanged()));

      if (scale.getNbNotes() == Scale::NB_ALL_NOTES)
            ui->checkShowAllNotes->setEnabled(false);
      }

//---------------------------------------------------------
//   EditNotesDialog d'tor
//---------------------------------------------------------

EditNotesDialog::~EditNotesDialog()
      {
      delete ui;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditNotesDialog::accept()
      {
      if (noteEdited) {
            updateNotes();
            ScaleParams from, to;
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  from.notes[i] = notes[i];
                  to.notes[i] = "";
                  }
            from.nbNotes = scale.getNbNotes();
            from.storingMode = storingMode;
            from.storeFifths = false;
            from.reference = reference;

            to.nbNotes = from.nbNotes;
            to.storingMode = Scale::ABSOLUTE_CENTS;
            to.storeFifths = false;
            to.reference = reference;

            Scale::recomputeNotes(from, to);
            scale = Scale(to);
            }
      else
            scale = originalScale;
      QDialog::accept();
      }

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void EditNotesDialog::reject()
      {
      scale = originalScale;
      QDialog::reject();
      }

//---------------------------------------------------------
//   showData
//---------------------------------------------------------

void EditNotesDialog::showData()
      {
      if (scale.getNbNotes() != originalScale.getNbNotes())
            ui->checkShowAllNotes->setCheckState(Qt::Checked);
      else
            ui->checkShowAllNotes->setCheckState(Qt::Unchecked);

      ui->comboStoringMode->setCurrentIndex(storingMode);

      float aNoteValue = scale.convertNoteValue(notes[TPC_A - TPC_MIN], TPC_A, storingMode, false);
      ui->comboReference->setCurrentIndex(aNoteValue == 0);

      showNotes();
      }

//---------------------------------------------------------
//   combosChanged
//---------------------------------------------------------

bool EditNotesDialog::combosChanged()
      {
      bool showAllNotes = ui->checkShowAllNotes->checkState() == Qt::Checked;
      if (showAllNotes != (scale.getNbNotes() != originalScale.getNbNotes()))
            return true;

      if (ui->comboStoringMode->currentIndex() != storingMode)
            return true;

      float aNoteValue = scale.convertNoteValue(notes[TPC_A - TPC_MIN], TPC_A, storingMode, false);
      if (ui->comboReference->currentIndex() != (aNoteValue == 0)) {
            return true;
            }

      return false;
      }

//---------------------------------------------------------
//   showNotes
//---------------------------------------------------------

void EditNotesDialog::showNotes()
      {
      updateNotes();
      if (combosChanged()) {
            validator->setStoringMode(ui->comboStoringMode->currentIndex());
            ScaleParams from, to;
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  from.notes[i] = notes[i];
                  to.notes[i] = "";
                  }
            from.nbNotes = scale.getNbNotes();
            from.storingMode = storingMode;
            from.storeFifths = false;
            from.reference = reference;

            if (ui->checkShowAllNotes->checkState() == Qt::Checked)
                  to.nbNotes = Scale::NB_ALL_NOTES;
            else
                  to.nbNotes = originalScale.getNbNotes();
            to.storingMode = ui->comboStoringMode->currentIndex();
            to.storeFifths = false;
            to.reference = ui->comboReference->currentIndex();

            Scale::recomputeNotes(from, to);

            scale = Scale(to);
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  notes[i] = to.notes[i];
                  }
            storingMode = to.storingMode;
            reference = ui->comboReference->currentIndex();
            }

      for (int tpc = scale.getMinTpc(); tpc <= scale.getMaxTpc(); ++tpc)
            noteValues[tpc - TPC_MIN]->setText(notes[tpc - TPC_MIN]);

      // Make the rest invisible
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc) {
            noteValues[tpc - TPC_MIN]->setVisible(
                  tpc >= scale.getMinTpc() && tpc <= scale.getMaxTpc());
            labels[tpc - TPC_MIN]->setVisible(
                  tpc >= scale.getMinTpc() && tpc <= scale.getMaxTpc());
            }
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void EditNotesDialog::updateNotes()
      {
      for (int tpc = scale.getMinTpc(); tpc <= scale.getMaxTpc(); ++tpc) {
            if (!noteValues[tpc - TPC_MIN]->text().isEmpty())
                  notes[tpc - TPC_MIN] = noteValues[tpc - TPC_MIN]->text();
            }
      }

//---------------------------------------------------------
//   noteChanged
//---------------------------------------------------------

void EditNotesDialog::noteChanged()
      {
      noteEdited = true;
      }

//---------------------------------------------------------
//   initNoteValuesArray
//---------------------------------------------------------

void EditNotesDialog::initNoteValuesArray()
      {
      noteValues[TPC_F_BB - TPC_MIN] = ui->lineFbb;
      noteValues[TPC_C_BB - TPC_MIN] = ui->lineCbb;
      noteValues[TPC_G_BB - TPC_MIN] = ui->lineGbb;
      noteValues[TPC_D_BB - TPC_MIN] = ui->lineDbb;
      noteValues[TPC_A_BB - TPC_MIN] = ui->lineAbb;
      noteValues[TPC_E_BB - TPC_MIN] = ui->lineEbb;
      noteValues[TPC_B_BB - TPC_MIN] = ui->lineBbb;

      noteValues[TPC_F_B - TPC_MIN] = ui->lineFb;
      noteValues[TPC_C_B - TPC_MIN] = ui->lineCb;
      noteValues[TPC_G_B - TPC_MIN] = ui->lineGb;
      noteValues[TPC_D_B - TPC_MIN] = ui->lineDb;
      noteValues[TPC_A_B - TPC_MIN] = ui->lineAb;
      noteValues[TPC_E_B - TPC_MIN] = ui->lineEb;
      noteValues[TPC_B_B - TPC_MIN] = ui->lineBb;

      noteValues[TPC_F - TPC_MIN] = ui->lineF;
      noteValues[TPC_C - TPC_MIN] = ui->lineC;
      noteValues[TPC_G - TPC_MIN] = ui->lineG;
      noteValues[TPC_D - TPC_MIN] = ui->lineD;
      noteValues[TPC_A - TPC_MIN] = ui->lineA;
      noteValues[TPC_E - TPC_MIN] = ui->lineE;
      noteValues[TPC_B - TPC_MIN] = ui->lineB;

      noteValues[TPC_F_S - TPC_MIN] = ui->lineFs;
      noteValues[TPC_C_S - TPC_MIN] = ui->lineCs;
      noteValues[TPC_G_S - TPC_MIN] = ui->lineGs;
      noteValues[TPC_D_S - TPC_MIN] = ui->lineDs;
      noteValues[TPC_A_S - TPC_MIN] = ui->lineAs;
      noteValues[TPC_E_S - TPC_MIN] = ui->lineEs;
      noteValues[TPC_B_S - TPC_MIN] = ui->lineBs;

      noteValues[TPC_F_SS - TPC_MIN] = ui->lineFss;
      noteValues[TPC_C_SS - TPC_MIN] = ui->lineCss;
      noteValues[TPC_G_SS - TPC_MIN] = ui->lineGss;
      noteValues[TPC_D_SS - TPC_MIN] = ui->lineDss;
      noteValues[TPC_A_SS - TPC_MIN] = ui->lineAss;
      noteValues[TPC_E_SS - TPC_MIN] = ui->lineEss;
      noteValues[TPC_B_SS - TPC_MIN] = ui->lineBss;

      for (int i = 0; i < TPC_NUM_OF; ++i)
            noteValues[i]->setValidator(validator);
      }

//---------------------------------------------------------
//   initLabelsArray
//---------------------------------------------------------

void EditNotesDialog::initLabelsArray()
      {
      labels[TPC_F_BB - TPC_MIN] = ui->labelFbb;
      labels[TPC_C_BB - TPC_MIN] = ui->labelCbb;
      labels[TPC_G_BB - TPC_MIN] = ui->labelGbb;
      labels[TPC_D_BB - TPC_MIN] = ui->labelDbb;
      labels[TPC_A_BB - TPC_MIN] = ui->labelAbb;
      labels[TPC_E_BB - TPC_MIN] = ui->labelEbb;
      labels[TPC_B_BB - TPC_MIN] = ui->labelBbb;

      labels[TPC_F_B - TPC_MIN] = ui->labelFb;
      labels[TPC_C_B - TPC_MIN] = ui->labelCb;
      labels[TPC_G_B - TPC_MIN] = ui->labelGb;
      labels[TPC_D_B - TPC_MIN] = ui->labelDb;
      labels[TPC_A_B - TPC_MIN] = ui->labelAb;
      labels[TPC_E_B - TPC_MIN] = ui->labelEb;
      labels[TPC_B_B - TPC_MIN] = ui->labelBb;

      labels[TPC_F - TPC_MIN] = ui->labelF;
      labels[TPC_C - TPC_MIN] = ui->labelC;
      labels[TPC_G - TPC_MIN] = ui->labelG;
      labels[TPC_D - TPC_MIN] = ui->labelD;
      labels[TPC_A - TPC_MIN] = ui->labelA;
      labels[TPC_E - TPC_MIN] = ui->labelE;
      labels[TPC_B - TPC_MIN] = ui->labelB;

      labels[TPC_F_S - TPC_MIN] = ui->labelFs;
      labels[TPC_C_S - TPC_MIN] = ui->labelCs;
      labels[TPC_G_S - TPC_MIN] = ui->labelGs;
      labels[TPC_D_S - TPC_MIN] = ui->labelDs;
      labels[TPC_A_S - TPC_MIN] = ui->labelAs;
      labels[TPC_E_S - TPC_MIN] = ui->labelEs;
      labels[TPC_B_S - TPC_MIN] = ui->labelBs;

      labels[TPC_F_SS - TPC_MIN] = ui->labelFss;
      labels[TPC_C_SS - TPC_MIN] = ui->labelCss;
      labels[TPC_G_SS - TPC_MIN] = ui->labelGss;
      labels[TPC_D_SS - TPC_MIN] = ui->labelDss;
      labels[TPC_A_SS - TPC_MIN] = ui->labelAss;
      labels[TPC_E_SS - TPC_MIN] = ui->labelEss;
      labels[TPC_B_SS - TPC_MIN] = ui->labelBss;
      }
}