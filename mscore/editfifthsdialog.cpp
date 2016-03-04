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
        {TPC_F_B,  "Fb"},  {TPC_C_B,  "Cb"},  {TPC_G_B,  "Gb"},  {TPC_D_B,  "Db"},
        {TPC_A_B,  "Ab"},  {TPC_E_B,  "Eb"},  {TPC_B_B,  "Bb"},
        {TPC_F,    "F"},   {TPC_C,    "C"},   {TPC_G,    "G"},   {TPC_D,    "D"},
        {TPC_A,    "A"},   {TPC_E,    "E"},   {TPC_B,    "B"},
        {TPC_F_S,  "F#"},  {TPC_C_S,  "C#"},  {TPC_G_S,  "G#"},  {TPC_D_S,  "D#"},
        {TPC_A_S,  "A#"},  {TPC_E_S,  "E#"},  {TPC_B_S,  "B#"},
        {TPC_F_SS, "F##"}, {TPC_C_SS, "C##"}, {TPC_G_SS, "G##"}, {TPC_D_SS, "D##"},
        {TPC_A_SS, "A##"}, {TPC_E_SS, "E##"}, {TPC_B_SS, "B##"}};

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
      showData(true);
      noteEdited = false;

      connect(ui->checkShowAllNotes, SIGNAL(stateChanged(int)), this, SLOT(showDeltas()));
      connect(ui->comboStoringMode, SIGNAL(currentIndexChanged(int)), this, SLOT(showDeltas()));

      for (int i = 0; i < TPC_NUM_OF; ++i)
            connect(noteValues[i], SIGNAL(editingFinished()), this, SLOT(noteChanged()));

      if (scale.getNbNotes() == Scale::NB_ALL_NOTES)
            ui->checkShowAllNotes->setEnabled(false);
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
      if (noteEdited) {
            updateDeltas();
            ScaleParams from, to;
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  from.notes[i] = notes[i];
                  to.notes[i] = "";
                  }
            from.nbNotes = scale.getNbNotes();
            from.storingMode = storingMode;
            from.storeFifths = true;

            to.nbNotes = from.nbNotes;
            to.storingMode = Scale::ABSOLUTE_CENTS;
            to.storeFifths = false;
            to.reference = Scale::A_REFERENCE;

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

void EditFifthsDialog::reject()
      {
      scale = originalScale;
      QDialog::reject();
      }

//---------------------------------------------------------
//   showData
//---------------------------------------------------------

void EditFifthsDialog::showData(bool initialization)
      {
      if (scale.getNbNotes() != originalScale.getNbNotes())
            ui->checkShowAllNotes->setCheckState(Qt::Checked);
      else
            ui->checkShowAllNotes->setCheckState(Qt::Unchecked);

      ui->comboStoringMode->setCurrentIndex(storingMode);

      showDeltas(initialization);
      }

//---------------------------------------------------------
//   combosChanged
//---------------------------------------------------------

bool EditFifthsDialog::combosChanged()
      {
      bool showAllNotes = ui->checkShowAllNotes->checkState() == Qt::Checked;
      if (showAllNotes != (scale.getNbNotes() != originalScale.getNbNotes()))
            return true;

      if (ui->comboStoringMode->currentIndex() != storingMode)
            return true;

      return false;
      }

//---------------------------------------------------------
//   showDeltas
//---------------------------------------------------------

void EditFifthsDialog::showDeltas(bool initialization)
      {
      updateDeltas();
      if (initialization || combosChanged()) {
            ScaleParams from, to;
            for (int i = 0; i < TPC_NUM_OF; ++i) {
                  from.notes[i] = notes[i];
                  to.notes[i] = "";
                  }
            from.nbNotes = scale.getNbNotes();
            from.storingMode = storingMode;
            from.storeFifths = !initialization;

            if (ui->checkShowAllNotes->checkState() == Qt::Checked)
                  to.nbNotes = Scale::NB_ALL_NOTES;
            else
                  to.nbNotes = originalScale.getNbNotes();
            to.storingMode = ui->comboStoringMode->currentIndex();
            to.storeFifths = true;
            to.reference = Scale::A_REFERENCE;

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
      noteValues[scale.getMinTpc() - TPC_MIN]->setText("");
      noteValues[0]->setText(notes[scale.getMinTpc() - TPC_MIN]);

      // Make the rest invisible
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc) {
            noteValues[tpc - TPC_MIN]->setVisible(
                  tpc == TPC_MIN ||
                  (tpc > scale.getMinTpc() && tpc <= scale.getMaxTpc()));
            labels[tpc - TPC_MIN]->setVisible(
                  tpc >= scale.getMinTpc() && tpc <= scale.getMaxTpc());
            }
      lastLabel->setText(LABEL_STRINGS.find(scale.getMinTpc())->second);
      }

//---------------------------------------------------------
//   updateDeltas
//---------------------------------------------------------

void EditFifthsDialog::updateDeltas()
      {
      for (int tpc = scale.getMinTpc() + 1; tpc <= scale.getMaxTpc(); ++tpc) {
            if (!noteValues[tpc - TPC_MIN]->text().isEmpty())
                  notes[tpc - TPC_MIN] = noteValues[tpc - TPC_MIN]->text();
            }
      if (!noteValues[0]->text().isEmpty())
            notes[scale.getMinTpc() - TPC_MIN] = noteValues[0]->text();
      }

//---------------------------------------------------------
//   .565
//---------------------------------------------------------

void EditFifthsDialog::noteChanged()
      {
      noteEdited = true;
      }

//---------------------------------------------------------
//   initNotesArray
//---------------------------------------------------------

void EditFifthsDialog::initNotesArray()
      {
      QWidget* client = ui->scrollAreaWidgetContents;
      QVBoxLayout* layout = new QVBoxLayout(client);

      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc) {
            QLabel* label = new QLabel(
                  LABEL_STRINGS.find(tpc)->second, client);

            layout->addWidget(label);
            QLineEdit* line = new QLineEdit(client);
            line->setValidator(new QDoubleValidator(line));
            layout->addWidget(line);

            noteValues[(tpc + 1 - TPC_MIN) % TPC_NUM_OF] = line;
            labels[tpc - TPC_MIN] = label;
            }
      lastLabel = new QLabel(LABEL_STRINGS.find(TPC_MIN)->second);
      layout->addWidget(lastLabel);

      noteValues[0]->setEnabled(false);
      }
}