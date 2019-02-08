#ifndef NOTETWEAKERDIALOG_H
#define NOTETWEAKERDIALOG_H

#include <QDialog>

namespace Ui {
class NoteTweakerDialog;
}

namespace Ms {

class Staff;
class Note;
class Chord;

class NoteTweakerDialog : public QDialog
      {
      Q_OBJECT

      Staff* _staff;
      QList<Note*> noteList;

public:
      explicit NoteTweakerDialog(QWidget *parent = nullptr);
      ~NoteTweakerDialog();

      void setStaff(Staff* s);

signals:
      void notesChanged();

public slots:
      void setNoteOffTime();

private:
      void addChord(Chord* chord, int voice);
      void updateNotes();
      void clearNoteData();


      Ui::NoteTweakerDialog *ui;
      };

}

#endif // NOTETWEAKERDIALOG_H
