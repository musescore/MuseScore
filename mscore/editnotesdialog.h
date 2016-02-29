#ifndef EDITNOTESDIALOG_H
#define EDITNOTESDIALOG_H

#include <QDialog>
#include "libmscore/scale.h"

namespace Ui {
class EditNotesDialog;
}

namespace Ms {

class EditNotesDialog : public QDialog {
      Q_OBJECT

private slots:
      void accept();
      void reject();
      void showNotes();

public:
      explicit EditNotesDialog(Scale scale, QWidget *parent = 0);
      ~EditNotesDialog();

      Scale getScale() { return scale; }

private:
      void showData();
      bool combosChanged();
      void initNoteValuesArray();
      void initLabelsArray();
      void updateNotes();

      Ui::EditNotesDialog *ui;
      Scale scale, originalScale;
      QLineEdit* (noteValues[TPC_NUM_OF]);
      QLabel* (labels[TPC_NUM_OF]);

      static const int     COMBO_SCALE_OPTIONS[Scale::NB_SCALES];

      int storingMode;
      QString notes[TPC_NUM_OF];
};

}

#endif // EDITNOTESDIALOG_H
