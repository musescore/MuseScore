#ifndef EDITFIFTHSDIALOG_H
#define EDITFIFTHSDIALOG_H

#include <QDialog>
#include "libmscore/scale.h"

namespace Ui {
class EditFifthsDialog;
}

namespace Ms {

class EditFifthsDialog : public QDialog {
      Q_OBJECT

private slots:
      void accept();
      void reject();
      void showDeltas(bool initialization = false);
      void noteChanged();

public:
      explicit EditFifthsDialog(Scale scale, QWidget *parent = 0);
      ~EditFifthsDialog();

      Scale getScale() { return scale; }

private:
      void showData(bool initialization = false);
      bool combosChanged();
      void initNotesArray();
      void updateDeltas();

      Ui::EditFifthsDialog *ui;
      Scale scale, originalScale;
      QLineEdit* (noteValues[TPC_NUM_OF]);
      QLabel* (labels[TPC_NUM_OF]);
      QLabel* lastLabel;

      static const int COMBO_SCALE_OPTIONS[Scale::NB_SCALES];
      static const std::map<int, QString> LABEL_STRINGS;

      bool noteEdited;
      int storingMode;
      QString notes[TPC_NUM_OF];
};

}

#endif // EDITFIFTHSDIALOG_H
