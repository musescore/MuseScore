#include "pianolevelschooser.h"
#include "pianolevelsfilter.h"

#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

PianoLevelsChooser::PianoLevelsChooser(QWidget *parent)
      : QWidget(parent)
{
      setupUi(this);

      _levelsIndex = 0;

      for (int i = 0; PianoLevelsFilter::FILTER_LIST[i]; ++i) {
            QString name = PianoLevelsFilter::FILTER_LIST[i]->name();
            levelsCombo->addItem(name, i);
            }

      connect(levelsCombo, SIGNAL(activated(int)), SLOT(setLevelsIndex(int)));
      connect(setEventsBn, SIGNAL(clicked(bool)), SLOT(setEventDataPressed()));
}

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoLevelsChooser::setStaff(Staff* staff)
      {
      _staff = staff;
      }


//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

void PianoLevelsChooser::setLevelsIndex(int index)
{
      if (_levelsIndex != index) {
            _levelsIndex = index;
            emit levelsIndexChanged(index);
            }
}


//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

void PianoLevelsChooser::setEventDataPressed()
      {
      PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];

      int val = eventValSpinBox->value();
      QList<Note*> noteList = _staff->getNotes();

      Score* score = _staff->score();

      score->startCmd();

      for(Note* note: noteList) {
            if (!note->selected())
                  continue;

            if (filter->isPerEvent()) {
                  for (NoteEvent& e : note->playEvents()) {
                        filter->setValue(_staff, note, &e, val);
                        }
                  }
                  else
                        filter->setValue(_staff, note, nullptr, val);

            }

      score->endCmd();

      emit notesChanged();
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

//void PianoLevelsChooser::addChord(Chord* chord, int voice)
//      {
//      for (Chord* c : chord->graceNotes())
//            addChord(c, voice);
//      for (Note* note : chord->notes()) {
//            if (note->tieBack())
//                  continue;
//            noteList.append(note);
//            }
//      }

}
