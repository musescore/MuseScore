#ifndef SCALESDIALOG_H
#define SCALESDIALOG_H

#include <QDialog>
#include "musescore.h"
#include "libmscore/scale.h"

namespace Ui {
class ScalesDialog;
}

namespace Ms {

class ScalesDialog : public QDialog {
      Q_OBJECT

private slots:
      void editAsFifths();
      void editAsNotes();
      void updateScaleName();
      void restoreDefaults();
      void importScalaFile();
      void updatePitches();

public:
      explicit ScalesDialog(Scale scale, QWidget *parent = 0);
      ~ScalesDialog();

      Scale getScale() { return scale; }

private:
      void showData();
      void showScaleName();

      MuseScore* mscore;
      Ui::ScalesDialog *ui;
      Scale scale;
};

}

#endif // SCALESDIALOG_H
