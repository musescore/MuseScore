#include "editfifthsdialog.h"
#include "ui_editfifthsdialog.h"
#include <QVBoxLayout>
#include <QLineEdit>

namespace Ms {

using namespace std;

const int EditFifthsDialog::COMBO_SCALE_OPTIONS[Scale::NB_SCALES] = { 7, 12, 17, 21, 35 };

const map<int, QString> EditFifthsDialog::LABEL_STRINGS =
      { {TPC_F_BB, "Fbb"}, {TPC_C_BB, "Cbb"}, {TPC_G_BB, "Gbb"}, {TPC_D_BB, "Dbb"},
            {TPC_A_BB, "Abb"}, {TPC_E_BB, "Ebb"}, {TPC_B_BB, "Bbb"},
            {TPC_F_B, "Fb"}, {TPC_C_B, "Cb"}, {TPC_G_B, "Gb"}, {TPC_D_B, "Db"},
            {TPC_A_B, "Ab"}, {TPC_E_B, "Eb"}, {TPC_B_B, "Bb"},
            {TPC_F, "F"}, {TPC_C, "C"}, {TPC_G, "G"}, {TPC_D, "D"},
            {TPC_A, "A"}, {TPC_E, "E"}, {TPC_B, "B"},
            {TPC_F_S, "Fs"}, {TPC_C_S, "Cs"}, {TPC_G_S, "Gs"}, {TPC_D_S, "Ds"},
            {TPC_A_S, "As"}, {TPC_E_S, "Es"}, {TPC_B_S, "Bs"},
            {TPC_F_SS, "Fss"}, {TPC_C_SS, "Css"}, {TPC_G_SS, "Gss"}, {TPC_D_SS, "Dss"},
            {TPC_A_SS, "Ass"}, {TPC_E_SS, "Ess"}, {TPC_B_SS, "Bss"}};

//---------------------------------------------------------
//   EditFifthsDialog c'tor
//---------------------------------------------------------

EditFifthsDialog::EditFifthsDialog(Scale scale, QWidget *parent) :
      QDialog(parent),
      ui(new Ui::EditFifthsDialog)
      {
      ui->setupUi(this);

      this->scale = originalScale = scale;
      ui->editFifthsLabel->setText(scale.getName());

      this->storingMode = Scale::ABSOLUTE_CENTS;
      QString* originalNotes = scale.getOriginalNotes();
      for (int i = 0; i < TPC_NUM_OF; ++i)
            this->notes[i] = originalNotes[i];

      initNotesArray();
      showData();

      connect(ui->comboScale, SIGNAL(currentIndexChanged(int)), this, SLOT(showDeltas()));
      connect(ui->comboStoringMode, SIGNAL(currentIndexChanged(int)), this, SLOT(showDeltas()));
      }

//---------------------------------------------------------
//   EditFifthsDialog d'tor
//---------------------------------------------------------

EditFifthsDialog::~EditFifthsDialog()
      {
      delete ui;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditFifthsDialog::accept()
      {
      updateDeltas();
      ScaleParams from, to;
      for (int i = 0; i < TPC_NUM_OF; ++i) {
            from.notes[i] = notes[i];
            to.notes[i] = "";
            }
      from.nbNotes = scale.getNbNotes();
      from.storingMode = storingMode;
      from.storeFifths = true;
      from.aTuning = scale.getAtuning();

      to.nbNotes = from.nbNotes;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      to.storeFifths = true;
      to.aTuning = "";
            
      Scale::recomputeNotes(from, to);
            
      scale = Scale(to);
      QDialog::accept();
      }

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void EditFifthsDialog::reject()
      {
      scale = originalScale;
      QDialog::reject();
      }

//---------------------------------------------------------
//   showData
//---------------------------------------------------------

void EditFifthsDialog::showData()
      {
      for (int index = 0; index < Scale::NB_SCALES; ++index)
            if (COMBO_SCALE_OPTIONS[index] == scale.getNbNotes())
                  ui->comboScale->setCurrentIndex(index);

      ui->comboStoringMode->setCurrentIndex(storingMode);

      showDeltas();
      }

//---------------------------------------------------------
//   combosChanged
//---------------------------------------------------------

bool EditFifthsDialog::combosChanged()
      {
      if (COMBO_SCALE_OPTIONS[ui->comboScale->currentIndex()] != scale.getNbNotes())
            return true;

      if (ui->comboStoringMode->currentIndex() != storingMode)
            return true;

      if (!scale.getStoreFifths())
            return true;

      return false;
      }

//---------------------------------------------------------
//   showDeltas
//---------------------------------------------------------

void EditFifthsDialog::showDeltas()
      {
      updateDeltas();
      if (combosChanged()) {
            ScaleParams from, to;
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  from.notes[i] = notes[i];
                  to.notes[i] = "";
                  }
            from.nbNotes = scale.getNbNotes();
            from.storingMode = storingMode;
            from.storeFifths = scale.getStoreFifths();
            from.aTuning = scale.getAtuning();

            to.nbNotes = COMBO_SCALE_OPTIONS[ui->comboScale->currentIndex()];
            to.storingMode = ui->comboStoringMode->currentIndex();
            to.storeFifths = true;
            to.aTuning = "";

            Scale::recomputeNotes(from, to);

            scale = Scale(to);
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  notes[i] = to.notes[i];
                  }
            storingMode = to.storingMode;
            }

      int tpcItem = Scale::SUPPORTED_NBS.find(scale.getNbNotes())->second;
      for (int index = 0; index < scale.getNbNotes(); ++index) {
            int tpc = Scale::TPCS[tpcItem][index];
            noteValues[tpc - TPC_MIN]->setText(notes[tpc - TPC_MIN]);
            }

      // Make the rest invisible
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc) {
            noteValues[tpc - TPC_MIN]->setVisible(
                  tpc >= scale.getMinTpc() && tpc <= scale.getMaxTpc());
            labels[tpc - TPC_MIN]->setVisible(
                  tpc >= scale.getMinTpc() &&
                  tpc <= scale.getMaxTpc());
            }
      lastLabel->setText(LABEL_STRINGS.find(scale.getMaxTpc())->second);
      }

//---------------------------------------------------------
//   updateDeltas
//---------------------------------------------------------

void EditFifthsDialog::updateDeltas()
      {
      for (int tpc = scale.getMinTpc(); tpc <= scale.getMaxTpc(); ++tpc)
            if (!noteValues[tpc - TPC_MIN]->text().isEmpty())
                  notes[tpc - TPC_MIN] = noteValues[tpc - TPC_MIN]->text();
      }

//---------------------------------------------------------
//   initNotesArray
//---------------------------------------------------------

void EditFifthsDialog::initNotesArray()
      {
      QWidget* client = ui->scrollAreaWidgetContents;
      QVBoxLayout* layout = new QVBoxLayout(client);

      Scale fullScale;
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc) {
            QLabel* label = new QLabel(
                  LABEL_STRINGS.find(fullScale.prevNote(tpc))->second, client);

            layout->addWidget(label);
            QLineEdit* line = new QLineEdit(client);
            layout->addWidget(line);

            noteValues[tpc - TPC_MIN] = line;
            labels[tpc - TPC_MIN] = label;
            }
      lastLabel = new QLabel(LABEL_STRINGS.find(TPC_MAX)->second);
      layout->addWidget(lastLabel);
      }
}