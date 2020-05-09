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
            levelsCombo->setItemData(i, PianoLevelsFilter::FILTER_LIST[i]->tooltip(), Qt::ToolTipRole);
            }

      connect(levelsCombo, SIGNAL(activated(int)), SLOT(setLevelsIndex(int)));
      connect(setEventsBn, SIGNAL(clicked(bool)), SLOT(setEventDataPressed()));
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
      int randVal = 0; // initializing randVal as 0
      QList<Note*> noteList = _staff->getNotes();

      Score* score = _staff->score();

      score->startCmd();

      // initialize random seed:
      std::srand(eventSeedSpinBox->value());

      for (Note* note: noteList) {
            if (!note->selected())
                  continue;

            // Generating a random integer from 0 to eventRandSpinBox->value()
            if (eventRandSpinBox->value() > 0) // else it remains 0, as initialized
            {
                randVal = std::rand()/((RAND_MAX + 1u)/eventRandSpinBox->value());
            }

            // applying set to selected notes:
            if (filter->isPerEvent()) {
                  for (NoteEvent& e : note->playEvents()) {
                      filter->setValue(_staff, note, &e, val + randVal);
                   }
            }
            else
                      filter->setValue(_staff, note, nullptr, val + randVal);

            }

      score->endCmd();

      emit notesChanged();
      }

}
